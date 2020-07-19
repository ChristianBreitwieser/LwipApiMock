//
// Created by christianb on 18.07.20.
//
#include <lwip/tcp.h>
extern "C" {

const ip_addr_t ip_addr_any = {0x0};
const ip_addr_t ip_addr_broadcast = {0xFFFFFFFF};

const char *lwip_strerr(err_t err) {
    return "";
}

}