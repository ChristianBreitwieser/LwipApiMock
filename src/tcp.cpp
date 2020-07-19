//
// Created by christianb on 18.07.20.
//

#include <lwip/tcp.h>
#include <tcp_asio.h>
#include <iostream>
#include <cstring>
#include <map>

std::map<struct tcp_pcb *, tcp_asio_pcb *> pcb_lookup;

extern "C" {

struct tcp_pcb * tcp_new     (void) {
    std::clog << "tcp_new()" << std::endl;


    struct tcp_pcb * new_pcb=new struct tcp_pcb;

    new_pcb->snd_queuelen=0;
    new_pcb->snd_buf=TCP_SND_BUF;

    auto asio_pcb = tcp_asio_new(new_pcb,TCP_MSS*2,new_pcb->snd_buf);
    pcb_lookup.insert({new_pcb,asio_pcb});

    return new_pcb;
}

err_t            tcp_bind    (struct tcp_pcb *pcb, const ip_addr_t *ipaddr,
                              u16_t port) {
    std::clog << "tcp_bind(" << pcb << "," << ipaddr <<"," <<port<<")"<< std::endl;
    auto found=pcb_lookup.find(pcb);
    if(found!=pcb_lookup.end()){
        pcb->local_port=port;
        pcb->local_ip=*ipaddr;
        return tcp_asio_bind(found->second,ipaddr->addr,port);
    }
    return ERR_VAL;

}


struct tcp_pcb * tcp_listen_with_backlog(struct tcp_pcb *pcb, u8_t backlog) {
    std::clog << "tcp_listen_with_backlog(" << pcb <<","<<backlog<<")"<< std::endl;

    auto found=pcb_lookup.find(pcb);
    if(found!=pcb_lookup.end()){
       tcp_asio_listen(found->second);
       pcb->state = LISTEN;
       return pcb;
    }
    return nullptr;
}

err_t tcp_write(struct tcp_pcb *pcb, const void *dataptr, u16_t len,
                u8_t apiflags) {
    std::clog << "tcp_write(" << pcb <<","<<dataptr<<","<<len<<","<<apiflags<<")"<< std::endl;
    auto found=pcb_lookup.find(pcb);
    if(found!=pcb_lookup.end()){
        bool push = (apiflags & TCP_WRITE_FLAG_MORE) == TCP_WRITE_FLAG_MORE;
        if(len>pcb->snd_buf){
            return ERR_MEM;
        }
        auto err=tcp_asio_write(found->second,dataptr,len,push);
        if(err==ERR_OK){
            pcb->snd_buf-=len; //decrement the available send_buffer
        }
        return err;
    }
}

err_t            tcp_output  (struct tcp_pcb *pcb) {
    std::clog << "tcp_output(" << pcb <<")"<< std::endl;
    auto found=pcb_lookup.find(pcb);
    if(found!=pcb_lookup.end()){
        return tcp_asio_output(found->second);
    }
}

void tcp_recved(struct tcp_pcb *pcb, u16_t len) {
    std::clog << "tcp_recved(" << pcb <<","<<len<<")"<< std::endl;
    auto found=pcb_lookup.find(pcb);
    if(found!=pcb_lookup.end()){
        tcp_asio_recved(found->second,len);
    }
}

void tcp_setprio (struct tcp_pcb *pcb, u8_t prio){
    std::clog << "tcp_setprio(" << pcb <<","<<prio<<")"<< std::endl;
    pcb->prio=prio;
}

void tcp_arg(struct tcp_pcb *pcb, void *arg) {
    std::clog << "tcp_arg(" << pcb <<","<<arg<<")"<< std::endl;
    pcb->callback_arg=arg;
}

static int8_t tcp_accept_intermediate(void * tcp_pcb,tcp_asio_pcb *newpcb, int8_t err){
    auto acceptor_pcb=((struct tcp_pcb_listen *)tcp_pcb);
    if(acceptor_pcb==nullptr || acceptor_pcb->accept==nullptr){
        tcp_asio_close(newpcb);
        return ERR_VAL;
    }

    struct tcp_pcb * lwip_pcb=new struct tcp_pcb;

    //set sendbuffer size
    lwip_pcb->snd_buf=TCP_SND_BUF;
    lwip_pcb->snd_queuelen=0;

    tcp_asio_set_pcb(newpcb,lwip_pcb);
    pcb_lookup.insert({lwip_pcb,newpcb});
    std::clog << "tcp_cb_accept()" << std::endl;
    return acceptor_pcb->accept(acceptor_pcb->callback_arg,lwip_pcb,err);
}

void             tcp_accept  (struct tcp_pcb *pcb,
                              err_t (* accept)(void *arg, struct tcp_pcb *newpcb,
                                               err_t err)){
    std::clog << "tcp_accept(" << pcb <<","<<accept<<")"<< std::endl;
    ((struct tcp_pcb_listen*)pcb)->accept=accept;

    auto found=pcb_lookup.find(pcb);
    if(found!=pcb_lookup.end()){
        auto cb_ptr= tcp_accept_intermediate;
        if(accept == nullptr){
            cb_ptr=nullptr;
        }
        tcp_asio_accept(found->second,cb_ptr);
    }
}

static int8_t tcp_recv_intermediate(void *tpcb,void  * data, uint16_t size, int8_t err){
    auto tcp_pcb=((struct tcp_pcb *)tpcb);
    if(tcp_pcb==nullptr || tcp_pcb->recv==nullptr){
        return ERR_VAL;
    }
    struct pbuf * p = pbuf_alloc(PBUF_RAW,size,PBUF_POOL);
    struct pbuf * buffer=p;
    auto remaining_length=size;
    while(buffer!=NULL){
        auto copylen = (buffer->len<remaining_length)?buffer->len:remaining_length;
        remaining_length-=copylen;
        std::memcpy(buffer->payload,data,copylen);
        buffer=buffer->next;
    }


    std::clog << "tcp_cb_recv()" << std::endl;
    return tcp_pcb->recv(tcp_pcb->callback_arg,tcp_pcb,p,err);
}

void tcp_recv(struct tcp_pcb *pcb,
              err_t (*recv)(void *arg, struct tcp_pcb *tpcb,
                            struct pbuf *p, err_t err)) {
    std::clog << "tcp_recv(" << pcb <<","<<recv<<")"<< std::endl;
    pcb->recv=recv;

    auto found=pcb_lookup.find(pcb);
    if(found!=pcb_lookup.end()){
        auto cb_ptr= tcp_recv_intermediate;
        if(recv == nullptr){
            cb_ptr=nullptr;
        }
        tcp_asio_recv(found->second,cb_ptr);
    }
}

static int8_t tcp_sent_intermediate(void * tpcb,uint16_t length){
    auto tcp_pcb=((struct tcp_pcb *)tpcb);
    if(tcp_pcb==nullptr || tcp_pcb->sent==nullptr){
        return ERR_VAL;
    }
    tcp_pcb->snd_buf+=length; //Increment the available send_buffer
    std::clog << "tcp_cb_sent()" << std::endl;
    return tcp_pcb->sent(tcp_pcb->callback_arg,tcp_pcb,length);
}

void tcp_sent(struct tcp_pcb *pcb,
              err_t (*sent)(void *arg, struct tcp_pcb *tpcb,
                            u16_t len)) {
    std::clog << "tcp_sent(" << pcb <<","<<sent<<")"<< std::endl;
    pcb->sent=sent;

    auto found=pcb_lookup.find(pcb);
    if(found!=pcb_lookup.end()){
        auto cb_ptr= tcp_sent_intermediate;
        if(sent == nullptr){
            cb_ptr=nullptr;
        }
        tcp_asio_sent(found->second,cb_ptr);
    }
}

static int8_t tcp_poll_intermediate(void * tpcb){
    auto tcp_pcb=((struct tcp_pcb *)tpcb);
    if(tcp_pcb==nullptr || tcp_pcb->poll==nullptr){
        return ERR_VAL;
    }

    std::clog << "tcp_cb_poll()" << std::endl;
    return tcp_pcb->poll(tcp_pcb->callback_arg,tcp_pcb);
}

void tcp_poll(struct tcp_pcb *pcb,
              err_t (*poll)(void *arg, struct tcp_pcb *tpcb),
              u8_t interval) {
    std::clog << "tcp_poll(" << pcb <<","<<poll<<","<<interval<<")"<< std::endl;
    pcb->poll=poll;
    pcb->pollinterval=interval;

    auto found=pcb_lookup.find(pcb);
    if(found!=pcb_lookup.end()){
        auto cb_ptr= tcp_poll_intermediate;
        if(poll == nullptr){
            cb_ptr=nullptr;
        }
        tcp_asio_poll(found->second,cb_ptr,interval);
    }
}

static int8_t tcp_err_intermediate(void * tpcb,int8_t err){
    auto tcp_pcb=((struct tcp_pcb *)tpcb);
    if(tcp_pcb==nullptr || tcp_pcb->errf==nullptr){
        return ERR_VAL;
    }

    std::clog << "tcp_cb_err()" << std::endl;
    tcp_pcb->errf(tcp_pcb->callback_arg,err);
    return ERR_OK;
}

void tcp_err(struct tcp_pcb *pcb,
             void (*err)(void *arg, err_t err)) {
    std::clog << "tcp_err(" << pcb <<","<<err<<")"<< std::endl;
    pcb->errf=err;

    auto found=pcb_lookup.find(pcb);
    if(found!=pcb_lookup.end()){
        auto cb_ptr= tcp_err_intermediate;
        if(err == nullptr){
            cb_ptr=nullptr;
        }
        tcp_asio_err(found->second,cb_ptr);
    }
}

void             tcp_abandon (struct tcp_pcb *pcb, int reset){
    std::clog << "tcp_abandon(" << pcb << ","<<reset<<")"<< std::endl;
    auto found=pcb_lookup.find(pcb);
    if(found!=pcb_lookup.end()){
        tcp_asio_close(found->second);
    }
    pcb_lookup.erase(pcb);
    delete pcb;
}

void             tcp_abort (struct tcp_pcb *pcb){
    std::clog << "tcp_abort(" << pcb << ")"<< std::endl;
    auto found=pcb_lookup.find(pcb);
    if(found!=pcb_lookup.end()){
        tcp_asio_close(found->second);
    }
    pcb_lookup.erase(pcb);
    delete pcb;

}

err_t            tcp_close   (struct tcp_pcb *pcb){
    std::clog << "tcp_close(" << pcb << ")"<< std::endl;
    auto found=pcb_lookup.find(pcb);
    if(found!=pcb_lookup.end()){
        tcp_asio_close(found->second);
    }
    pcb_lookup.erase(pcb);
    delete pcb;
    return ERR_OK;
}

const char* tcp_debug_state_str(enum tcp_state s) {

}

}