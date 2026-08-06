#include "tcp_session.h"

#include <utility>

#include <boost/asio/buffer.hpp>
#include <boost/asio/write.hpp>
#include <boost/system/error_code.hpp>

#include "command_processor.h"

TcpSession::TcpSession(boost::asio::ip::tcp::socket socket,
                       CommandProcessor& command_processor)
    : m_socket(std::move(socket)), m_CommandProcessor(command_processor) {}

void TcpSession::start() { read(); }

void TcpSession::read() {
    std::shared_ptr<TcpSession> self = shared_from_this();
    m_socket.async_read_some(
        boost::asio::buffer(m_buffer),
        [self](const boost::system::error_code& error, std::size_t received) {
            if (received > 0) {
                self->m_Input.append(self->m_buffer.data(), received);
                self->process_commands();
            }

            if (error) {
                return;
            }

            self->read();
        });
}

void TcpSession::process_commands() {
    std::size_t delimiter = m_Input.find('\n');
    while (delimiter != std::string::npos) {
        const std::string command = m_Input.substr(0, delimiter);
        m_Input.erase(0, delimiter + 1);

        const bool write_in_progress = !m_Responses.empty();
        m_Responses.push_back(m_CommandProcessor.process(command));
        if (!write_in_progress) {
            write();
        }

        delimiter = m_Input.find('\n');
    }
}

void TcpSession::write() {
    std::shared_ptr<TcpSession> self = shared_from_this();
    boost::asio::async_write(
        m_socket,
        boost::asio::buffer(m_Responses.front()),
        [self](const boost::system::error_code& error, std::size_t) {
            if (error) {
                return;
            }

            self->m_Responses.pop_front();
            if (!self->m_Responses.empty()) {
                self->write();
            }
        });
}
