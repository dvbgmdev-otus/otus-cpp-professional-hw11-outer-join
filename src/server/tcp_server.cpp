#include "tcp_server.h"

#include <memory>
#include <utility>

#include <boost/system/error_code.hpp>

#include "tcp_session.h"

TcpServer::TcpServer(boost::asio::io_context& io_context,
                     std::uint16_t port,  // NOLINT(bugprone-easily-swappable-parameters)
                     std::size_t block_size)
    : m_acceptor(io_context,
                 boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
      m_block_size(block_size) {}

void TcpServer::start() { accept(); }

void TcpServer::accept() {
    m_acceptor.async_accept(
        [this](const boost::system::error_code& error, boost::asio::ip::tcp::socket socket) {
            if (!error) {
                std::make_shared<TcpSession>(std::move(socket), m_block_size)->start();
            }

            if (m_acceptor.is_open()) {
                accept();
            }
        });
}
