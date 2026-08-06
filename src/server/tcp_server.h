#ifndef TCP_SERVER_H
#define TCP_SERVER_H

/**
 * @file tcp_server.h
 * @brief Асинхронный TCP-сервер команд.
 */

#include <cstddef>
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
     * @param block_size Размер статического блока команд.
     */
    TcpServer(boost::asio::io_context& io_context,
              std::uint16_t port,
              std::size_t block_size);

    TcpServer(const TcpServer&) = delete;
    TcpServer& operator=(const TcpServer&) = delete;

    /// Запускает асинхронный приём подключений.
    void start();

private:
    /// Планирует следующую операцию accept.
    void accept();

    /// Acceptor входящих TCP-подключений.
    boost::asio::ip::tcp::acceptor m_acceptor;

    /// Размер статического блока команд.
    std::size_t m_block_size;
};

#endif  // TCP_SERVER_H
