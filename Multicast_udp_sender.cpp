#include "Multicast_udp_sender.hpp"

multicast_udp_sender::multicast_udp_sender(boost::asio::io_service &service,
                                           const boost::asio::ip::address &multicast_address, int port) : endpoint(
        multicast_address, port),
                                                                                                          socket(service,
                                                                                                                 endpoint.protocol()),
                                                                                                          timer(service) {}

void multicast_udp_sender::send() {
    pid_t pid = getpid();
    std::ostringstream os;
    os << pid;
    message = os.str();
    socket.async_send_to(boost::asio::buffer(message), endpoint,
                         std::bind(&multicast_udp_sender::send_to_handler, this, std::placeholders::_1,
                                   std::placeholders::_2));
}


void multicast_udp_sender::send_to_handler(const boost::system::error_code &code, size_t bytes) {
    if (code) {
        std::cerr << code.message() << std::endl;
    } else {
        std::cout << "Successfully send: '" + message << "' [" << bytes << "bytes]"
                  << std::endl;
        timer.expires_from_now(boost::posix_time::seconds(1));
        timer.async_wait(std::bind(&multicast_udp_sender::timer_handler, this, std::placeholders::_1));
    }
}

void multicast_udp_sender::timer_handler(const boost::system::error_code &code) {
    if (code) {
        std::cerr << code.message() << std::endl;
    } else {
        this->send();
    }
}

