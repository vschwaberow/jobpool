#pragma once

#include <boost/asio.hpp>
#include <string>
#include <future>

class DnsResolver {
public:
    DnsResolver() : io_context_(), resolver_(io_context_) {}

    std::future<std::string> resolve(const std::string& hostname) {
        return std::async(std::launch::async, [this, hostname]() {
            try {
                auto results = resolver_.resolve(hostname, "");
                if (!results.empty()) {
                    return results.begin()->endpoint().address().to_string();
                }
            }
            catch (const boost::system::system_error& e) {
                return std::string("Error: ") + e.what();
            }
            return std::string("No results");
        });
    }

private:
    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::resolver resolver_;
};
