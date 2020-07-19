//
// Created by christianb on 18.07.20.
//

#include <tcp_asio.h>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <errors.h>
#include <io_service.h>
#include <boost/shared_ptr.hpp>
#include <iostream>

using boost::asio::ip::tcp;

class tcp_asio_pcb : public boost::enable_shared_from_this<tcp_asio_pcb> {
public:
    void *tcp_pcb;
    uint16_t recvbuffer_size;
    uint16_t sndbuffer_size;

    std::unique_ptr<tcp::endpoint> endpoint;
    std::unique_ptr<tcp::acceptor> acceptor;
    std::unique_ptr<tcp::socket> socket;
    std::unique_ptr<boost::asio::high_resolution_timer> timer;
    std::unique_ptr<boost::asio::streambuf> streambuf_read;
    std::unique_ptr<boost::asio::streambuf> streambuf_write;

    int8_t (*accept_cb)(void *tcp_pcb, tcp_asio_pcb *newpcb, int8_t err);

    int8_t (*recv_cb)(void *tpcb, void *data, uint16_t size, int8_t err);

    int8_t (*sent_cb)(void *tpcb, uint16_t length);

    int8_t (*poll_cb)(void *tpcb);

    int8_t (*err_cb)(void *tpcb, int8_t err);

    tcp_asio_pcb(void *tcp_pcb, uint16_t recvbuffer, uint16_t sndbuffer) :
            tcp_pcb(tcp_pcb),
            recvbuffer_size(recvbuffer),
            sndbuffer_size(sndbuffer),
            endpoint(nullptr),
            acceptor(nullptr),
            socket(nullptr),
            timer(nullptr),
            streambuf_read(nullptr),
            accept_cb(nullptr),
            recv_cb(nullptr),
            sent_cb(nullptr),
            poll_cb(nullptr),
            err_cb(nullptr) {

    }

    void async_start_accept() {
        boost::shared_ptr<tcp_asio_pcb> newPCB(new tcp_asio_pcb(nullptr, recvbuffer_size, sndbuffer_size));
        newPCB->socket.reset(new tcp::socket(acceptor->get_io_service()));
        acceptor->async_accept(*newPCB->socket, boost::bind(&tcp_asio_pcb::async_handle_accept, this, newPCB,
                                                            boost::asio::placeholders::error));
    }

    void async_handle_accept(boost::shared_ptr<tcp_asio_pcb> pcb, const boost::system::error_code &error) {
        if (!error) {
            if (accept_cb != nullptr && accept_cb(tcp_pcb, pcb.get(), 0) == ERR_OK) {
                pcb->streambuf_read.reset(new boost::asio::streambuf(recvbuffer_size));
                pcb->streambuf_write.reset(new boost::asio::streambuf(sndbuffer_size));
                pcb->async_start_read();
            }
            async_start_accept();
        } else {
            std::cerr << error.message();
        }
    }

    void async_start_read() {
        if (streambuf_read && socket) {
            boost::asio::async_read(*socket, *streambuf_read, boost::asio::transfer_at_least(1),
                                    boost::bind(&tcp_asio_pcb::async_handle_read, shared_from_this(),
                                                boost::asio::placeholders::error,
                                                boost::asio::placeholders::bytes_transferred));
        }
    }

    void async_handle_read(const boost::system::error_code &error, std::size_t bytes_transferred) {
        auto data = streambuf_read->data();
        if (recv_cb != nullptr) {
            //beware - here is data's constness casted into the nirvana
            recv_cb(tcp_pcb, (void *) data.data(), (uint16_t) bytes_transferred, 0);
        } else {
            streambuf_read->consume(bytes_transferred);
        }
        async_start_read();
    }


    void async_start_write() {
        if (streambuf_write && socket) {
            boost::asio::async_write(*socket, *streambuf_write,
                                     boost::bind(&tcp_asio_pcb::async_handle_write, shared_from_this(),
                                                 boost::asio::placeholders::error,
                                                 boost::asio::placeholders::bytes_transferred));
        }
    }

    void async_handle_write(const boost::system::error_code &error, std::size_t bytes_transferred) {
        if (sent_cb != nullptr) {
            sent_cb(tcp_pcb, (uint16_t) bytes_transferred);
        }
    }

    void async_stop_poll_timer() {
        if (timer) {
            timer.reset(nullptr);
        }
    }

    void async_start_poll_timer(uint8_t interval) {
        if (!timer) {
            timer.reset(new boost::asio::high_resolution_timer(socket->get_io_context()));
        }

        timer->expires_after(500 * std::chrono::milliseconds(interval));
        timer->async_wait(boost::bind(&tcp_asio_pcb::async_handle_poll, shared_from_this(), interval,
                                      boost::asio::placeholders::error));
    }

    void async_handle_poll(uint8_t interval, const boost::system::error_code &error) {
        if (!error) {
            if (poll_cb != nullptr) {
                poll_cb(tcp_pcb);
            }
            if (timer) {
                async_start_poll_timer(interval);
            }
        }
    }


    void close() {
        if (acceptor) {
            acceptor->close();
        }
        if (socket) {
            socket->close();
        }
        if (timer) {
            timer->cancel();
        }
    }

    ~tcp_asio_pcb() {
        std::cerr << "Destructor called" << std::endl;
    }
};


tcp_asio_pcb *tcp_asio_new(void *tcp_pcb, uint16_t recvbuffer, uint16_t sndbuffer) {
    return new tcp_asio_pcb(tcp_pcb, recvbuffer, sndbuffer);
}

void tcp_asio_set_pcb(tcp_asio_pcb *pcb, void *tcp_pcb) {
    pcb->tcp_pcb = tcp_pcb;
}

boost::asio::ip::address convertAddress(uint32_t addr) {
    return boost::asio::ip::make_address_v4(addr);
}

int8_t tcp_asio_bind(tcp_asio_pcb *pcb, uint32_t ipaddr,
                     uint16_t port) {
    if (ipaddr == 0) {
        pcb->endpoint.reset(new tcp::endpoint(tcp::v4(), port));
    } else {
        pcb->endpoint.reset(new tcp::endpoint(convertAddress(ipaddr), port));
    }
    return ERR_OK;
}

void tcp_asio_listen(tcp_asio_pcb *pcb) {
    pcb->acceptor.reset(new tcp::acceptor(*GetIOService(), *(pcb->endpoint)));
    pcb->async_start_accept();
}

void tcp_asio_close(tcp_asio_pcb *pcb) {
    pcb->close();
    auto ptr = pcb->shared_from_this();
    //If only this instance existed it is now deleted .
}

int8_t tcp_asio_write(tcp_asio_pcb *pcb, void const *data_ptr, uint16_t len, bool push_flag) {
    (void) push_flag; //no way to control PSH flag from asio
    if (pcb->streambuf_write) {
        auto buffer = pcb->streambuf_write->prepare(len);
        if (len > buffer.size()) {
            return ERR_MEM;
        }
        std::memcpy(buffer.data(), data_ptr, len);
        pcb->streambuf_write->commit(len);
        return ERR_OK;
    }
    return ERR_VAL;
}

int8_t tcp_asio_output(tcp_asio_pcb *pcb) {
    pcb->async_start_write();
    return ERR_OK;
}

void tcp_asio_recved(tcp_asio_pcb *pcb, uint16_t len) {
    if (pcb->streambuf_read) {
        pcb->streambuf_read->consume(len);
    }
}

void tcp_asio_accept(tcp_asio_pcb *pcb,
                     int8_t (*accept)(void *tcp_pcb, tcp_asio_pcb *newpcb, int8_t err)) {
    pcb->accept_cb = accept;
}

void tcp_asio_recv(tcp_asio_pcb *pcb, int8_t (*recv)(void *tpcb, void *data, uint16_t size, int8_t err)) {
    pcb->recv_cb = recv;
}

void tcp_asio_sent(tcp_asio_pcb *pcb, int8_t (*sent)(void *tpcb, uint16_t length)) {
    pcb->sent_cb = sent;
}

void tcp_asio_poll(tcp_asio_pcb *pcb, int8_t (*poll)(void *tpcb), uint8_t interval) {
    pcb->poll_cb = poll;
    pcb->async_stop_poll_timer();
    if (poll != nullptr) {
        pcb->async_start_poll_timer(interval);
    }
}

void tcp_asio_err(tcp_asio_pcb *pcb, int8_t (*errf)(void *tpcb, int8_t err)) {
    pcb->err_cb = errf;
}