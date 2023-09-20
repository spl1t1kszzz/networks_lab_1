#include <iostream>
#include <unordered_map>
#include <boost/asio.hpp>
#include "Multicast_udp_sender.hpp"
#include "Multicast_udp_receiver.hpp"

constexpr auto port = 30001;

using namespace boost::asio::ip;

/*
 * Usage:
 *      for sender: 'send' <multicast_address>
 *      for receiver: 'receive' <listen_address> <multicast_address>
 *
 */
int main(int argc, char *argv[]) {
    try {
        if (argc < 3) {
            std::cerr << "Not enough args" << std::endl;
            return 1;
        } else {
            boost::asio::io_service io_service;
            if (strcmp(argv[1], "send") == 0) {
                multicast_udp_sender sender(io_service, boost::asio::ip::address::from_string(argv[2]), port);
                sender.send();
                io_service.run();
            } else if (strcmp(argv[1], "receive") == 0) {
                multicast_udp_receiver receiver(io_service, address::from_string(argv[2]),
                                                address::from_string(argv[3]), port, 5);
                receiver.wait();
                receiver.receive();
                io_service.run();

            }
        }
    }
    catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
