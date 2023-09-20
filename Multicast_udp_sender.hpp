#ifndef LAB_1_MULTICAST_UDP_SENDER_HPP
#define LAB_1_MULTICAST_UDP_SENDER_HPP

#include <boost/asio.hpp>
#include <iostream>
#include <functional>

/*
 * This class is designed to send multicast UDP packets
 */

using namespace boost::asio::ip;

class multicast_udp_sender {
private:
    udp::endpoint endpoint;
    udp::socket socket;
    boost::asio::deadline_timer timer;
    std::string message;

    void send_to_handler(const boost::system::error_code &code, size_t bytes);

    void timer_handler(const boost::system::error_code &code);

public:
    multicast_udp_sender(boost::asio::io_service &service, const address &multicast_address, int port);

    // sends the message, then waits for 1 second, then sends again
    void send();


};

#endif //LAB_1_MULTICAST_UDP_SENDER_HPP
