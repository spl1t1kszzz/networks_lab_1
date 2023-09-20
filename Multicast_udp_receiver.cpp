#include "Multicast_udp_receiver.hpp"

multicast_udp_receiver::multicast_udp_receiver(boost::asio::io_service &service, const address &listen_address,
                                               const address &multicast_address, int port, int ttw) :
        socket(service), timer(service) {
    this->time_to_wait = ttw;
    udp::endpoint endpoint(listen_address, port);
    this->socket.open(endpoint.protocol());
    this->socket.set_option(udp::socket::reuse_address(true));
    this->socket.bind(endpoint);
    this->socket.set_option(multicast::join_group(multicast_address));
}


void multicast_udp_receiver::print_alive_copies() {
    if (this->alive_copies.empty()) {
        std::cout << "no copies :(" << std::endl;
    } else {
        std::cout << "Alive copies: ";
        for (const auto &pair: this->alive_copies) {
            std::cout << pair.first << ' ';
        }
        std::cout << std::endl;
    }

}

bool isCopyOld(const std::pair<std::string, std::chrono::time_point<std::chrono::high_resolution_clock>> &item) {
    auto const &[key, value] = item;
    std::chrono::duration<double> elapsed_time = std::chrono::high_resolution_clock::now() - value;
    return elapsed_time.count() > 3;
}

void multicast_udp_receiver::wait() {
    std::erase_if(this->alive_copies, isCopyOld);
    print_alive_copies();
    this->timer.expires_from_now(boost::posix_time::seconds(this->time_to_wait));
    this->timer.async_wait(std::bind(&multicast_udp_receiver::timer_handler, this, std::placeholders::_1));
}

void multicast_udp_receiver::timer_handler(const boost::system::error_code &code) {
    if (code) {
        std::cerr << code.message() << std::endl;
    } else {
        this->wait();
    }
}

void multicast_udp_receiver::receive() {
    this->socket.async_receive_from(boost::asio::buffer(data, 256), sender_endpoint,
                                    std::bind(&multicast_udp_receiver::receive_from_handler, this,
                                              std::placeholders::_1, std::placeholders::_2));
}

void multicast_udp_receiver::receive_from_handler(const boost::system::error_code &code, size_t bytes) {
    if (code) {
        std::cerr << code.message() << std::endl;
    } else {
        std::string m(data);
        // std::cout << "Received: " + m << "[" << bytes << "bytes]" << std::endl;
        this->alive_copies[m] = std::chrono::high_resolution_clock::now();
        this->receive();
    }
}
