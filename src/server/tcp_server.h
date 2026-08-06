#ifndef TCP_SERVER_H
#define TCP_SERVER_H

/**
 * @file tcp_server.h
 * @brief Асинхронный TCP-сервер команд.
 */

#include <cstdint>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

/**
 * @brief Принимает произвольное количество TCP-подключений.
 * @ingroup main_group
 */
class TcpServer {
public:
    /**
     * @brief Создаёт сервер на заданном TCP-порту.
     *
     * @param io_context Контекст выполнения асинхронных операций Boost.ASIO.
     * @param port TCP-порт входящих подключений.
     */
    TcpServer(boost::asio::io_context& io_context, std::uint16_t port);

    TcpServer(const TcpServer&) = delete;
    TcpServer& operator=(const TcpServer&) = delete;

    /// Запускает асинхронный приём подключений.
    void start();

private:
    /// Планирует следующую операцию accept.
    void accept();

    /// Acceptor входящих TCP-подключений.
    boost::asio::ip::tcp::acceptor m_acceptor;
};

#endif  // TCP_SERVER_H
