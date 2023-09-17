#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <chrono>
#include "boost/bind.hpp"
#include <boost/asio.hpp>

constexpr auto port = 30001;
constexpr auto TTL = 5;

using namespace boost::asio::ip;


class multicast_udp_sender {
private:
    udp::endpoint endpoint;
    udp::socket socket;
    boost::asio::deadline_timer timer;
    std::string message;
public:
    multicast_udp_sender(boost::asio::io_service &ioService, const address &multicast_address) :
            endpoint(multicast_address, port), socket(ioService, endpoint.protocol()), timer(ioService) {
        socket.set_option(udp::socket::reuse_address(true));
        socket.set_option(multicast::join_group(multicast_address));

    };

    void send() {
        pid_t pid = getpid();
        std::ostringstream os;
        os << pid;
        message = os.str();
        socket.async_send_to(boost::asio::buffer(message), endpoint,
                             [this](const boost::system::error_code &e, size_t bytes) {
                                 if (e) {
                                     std::cerr << e.message() << std::endl;
                                 } else {
                                     std::cout << "Successfully send: '" + message << "' [" << bytes << "bytes]"
                                               << std::endl;
                                     timer.expires_from_now(boost::posix_time::seconds(1));
                                     timer.async_wait([this](const boost::system::error_code &code) {
                                         if (code) {
                                             std::cerr << code.message() << std::endl;
                                         } else {
                                             this->send();
                                         }
                                     });
                                 }
                             });
    }

};

class multicast_udp_receiver {
private:
    udp::socket socket;
    udp::endpoint sender_endpoint;
    char data[256];
    std::unordered_map<std::string, std::chrono::time_point<std::chrono::high_resolution_clock>> alive_copies;
    boost::asio::deadline_timer timer;
public:
    multicast_udp_receiver(boost::asio::io_service &service, const address &listen_address,
                           const address &multicast_address) :
            socket(service), timer(service) {

        udp::endpoint endpoint(listen_address, port);
        this->socket.open(endpoint.protocol());
        this->socket.set_option(udp::socket::reuse_address(true));
        this->socket.bind(endpoint);
        this->socket.set_option(multicast::join_group(multicast_address));
        this->get_alive_copies();
    }

    void get_alive_copies() {
        if (this->alive_copies.empty()) {
            std::cout << "no copies :(" << std::endl;
        } else {
            std::cout << "Alive copies: ";
            for (const auto &pair: this->alive_copies) {
                std::cout << pair.first << ' ';
            }
            std::cout << std::endl;
        }
        this->timer.expires_from_now(boost::posix_time::seconds(10));
        this->timer.async_wait([this](const boost::system::error_code &code) {
            if (code) {
                std::cerr << code.message() << std::endl;
            } else {
                std::erase_if(this->alive_copies, [](const auto &item) {
                    auto const &[key, value] = item;
                    std::chrono::duration<double> elapsed_time =
                            std::chrono::high_resolution_clock::now() - value;
                    return elapsed_time.count() > TTL;
                });
                this->get_alive_copies();
            }
        });
    }


    void receive() {
        this->socket.async_receive_from(boost::asio::buffer(data, 256), sender_endpoint,
                                  [this](const boost::system::error_code &code, size_t bytes) {
                                      if (code) {
                                          std::cerr << code.message() << std::endl;
                                      } else {
                                          std::string m(data);
                                          // std::cout << "Received: " + m << "[" << bytes << "bytes]" << std::endl;
                                          this->alive_copies[m] = std::chrono::high_resolution_clock::now();
                                          this->receive();
                                      }
                                  });
    }

};


// usage:
// send 224.0.0.1 or ff0e::1:2:3:4
// listen (0.0.0.0 or ::) (ff0e::1:2:3:4)
int main(int argc, char *argv[]) {
    try {
        if (argc < 3) {
            std::cerr << "Not enough args!" << std::endl;
            return 1;
        } else {
            boost::asio::io_service io_service;
            if (strcmp(argv[1], "send") == 0) {
                multicast_udp_sender sender(io_service, boost::asio::ip::address::from_string(argv[2]));
                sender.send();
                io_service.run();
            } else if (strcmp(argv[1], "listen") == 0) {
                multicast_udp_receiver receiver(io_service, address::from_string(argv[2]),
                                                address::from_string(argv[3]));
                receiver.receive();
                io_service.run();

            }
        }
    }
    catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;;
    }
    return 0;
}
