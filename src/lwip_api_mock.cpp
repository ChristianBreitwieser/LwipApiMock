#include <lwip_api_mock.h>

#include <iostream>
#include <boost/asio.hpp>
#include <io_service.h>

extern "C" {


void Start_lwipApplication(void (*App)(void)){
    try
    {
        boost::asio::io_service io_service;
        SetIOService(&io_service);
        if(App!=nullptr){
            App();
        }
        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}

}
