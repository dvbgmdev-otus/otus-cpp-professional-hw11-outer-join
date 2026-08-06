#include "tcp_session.h"

#include <utility>

#include <boost/asio/buffer.hpp>
#include <boost/system/error_code.hpp>

TcpSession::TcpSession(boost::asio::ip::tcp::socket socket, std::size_t block_size)
    : m_socket(std::move(socket)), m_handle(async::connect(block_size)) {}

TcpSession::~TcpSession() { disconnect(); }

void TcpSession::start() { read(); }

void TcpSession::read() {
    std::shared_ptr<TcpSession> self = shared_from_this();
    m_socket.async_read_some(
        boost::asio::buffer(m_buffer),
        [self](const boost::system::error_code& error, std::size_t received) {
            if (received > 0) {
                async::receive(self->m_handle, self->m_buffer.data(), received);
            }

            if (error) {
                self->disconnect();
                return;
            }

            self->read();
        });
}

void TcpSession::disconnect() {
    if (m_handle == nullptr) {
        return;
    }

    async::disconnect(m_handle);
    m_handle = nullptr;
}
