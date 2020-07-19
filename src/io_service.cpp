//
// Created by christianb on 18.07.20.
//

#include <io_service.h>

#include <memory>

boost::asio::io_service * service = nullptr;

void SetIOService(boost::asio::io_service * sv){
    service=sv;
}

boost::asio::io_service * GetIOService(void){
    return service;
}