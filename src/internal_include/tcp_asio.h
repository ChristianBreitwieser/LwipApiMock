//
// Created by christianb on 18.07.20.
//

#ifndef RTSP_PARSER_TCP_ASIO_H
#define RTSP_PARSER_TCP_ASIO_H
#include <cstdint>
class tcp_asio_pcb;

tcp_asio_pcb * tcp_asio_new(void * tcp_pcb, uint16_t recvbuffer, uint16_t sndbuffer);

void tcp_asio_set_pcb(tcp_asio_pcb * pcb,void * tcp_pcb);

void tcp_asio_close( tcp_asio_pcb * pcb);

int8_t tcp_asio_bind    (tcp_asio_pcb *pcb, uint32_t ipaddr,
             uint16_t port);

void tcp_asio_listen    (tcp_asio_pcb *pcb);

int8_t tcp_asio_write(tcp_asio_pcb *pcb,void const * data_ptr,uint16_t len,bool push);

int8_t tcp_asio_output(tcp_asio_pcb *pcb);

void tcp_asio_recved(tcp_asio_pcb *pcb, uint16_t len);

void tcp_asio_accept  (tcp_asio_pcb *pcb,
                              int8_t (* accept)(void * tcp_pcb,tcp_asio_pcb *newpcb, int8_t err));

void tcp_asio_recv(tcp_asio_pcb *pcb,int8_t (* recv)(void *tpcb,void  * data, uint16_t size, int8_t err));

void tcp_asio_sent(tcp_asio_pcb *pcb,int8_t (* sent)(void * tpcb,uint16_t length));

void tcp_asio_poll(tcp_asio_pcb *pcb,int8_t (* poll)(void * tpcb),uint8_t interval);

void tcp_asio_err(tcp_asio_pcb *pcb,int8_t (* errf)(void * tpcb,int8_t err));

#endif //RTSP_PARSER_TCP_ASIO_H
