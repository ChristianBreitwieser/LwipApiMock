//
// Created by christianb on 18.07.20.
//

#ifndef RTSP_PARSER_IO_SERVICE_H
#define RTSP_PARSER_IO_SERVICE_H
#include <boost/asio.hpp>

void SetIOService(boost::asio::io_service * sv);
boost::asio::io_service * GetIOService(void);

#endif //RTSP_PARSER_IO_SERVICE_H
