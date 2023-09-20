#ifndef LAB_1_MULTICAST_UDP_RECEIVER_HPP
#define LAB_1_MULTICAST_UDP_RECEIVER_HPP

#include <iostream>
#include <boost/asio.hpp>

/*
 * This class is designed to receive multicast UDP packets and
 * provide functionality to track and manage the sender's aliveness
 */


using namespace boost::asio::ip;

class multicast_udp_receiver {
private:
    udp::socket socket;
    udp::endpoint sender_endpoint;
    char data[256];
    // A map to track the sender's aliveness using timestamps
    std::unordered_map<std::string, std::chrono::time_point<std::chrono::high_resolution_clock>> alive_copies;
    boost::asio::deadline_timer timer;
    // Time to wait before print alive copies
    int time_to_wait;

    void receive_from_handler(const boost::system::error_code &, size_t);

    void timer_handler(const boost::system::error_code &);

public:
    //
    multicast_udp_receiver(boost::asio::io_service &service, const address &listen_address,
                           const address &multicast_address, int port, int ttw);

    // Prints the list of alive copies of the sender.
    void print_alive_copies();

    // Updates the list of alive copies and pauses for time_to_wait seconds
    void wait();

    // Initiates the reception of multicast packets
    void receive();

};

#endif //LAB_1_MULTICAST_UDP_RECEIVER_HPP
