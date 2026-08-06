#ifndef TCP_SESSION_H
#define TCP_SESSION_H

/**
 * @file tcp_session.h
 * @brief Асинхронная TCP-сессия одного клиента.
 */

#include <array>
#include <cstddef>
#include <memory>

#include <boost/asio/ip/tcp.hpp>

#include "async.h"

/**
 * @brief Читает команды одного TCP-клиента и передаёт их библиотеке async.
 * @ingroup main_group
 */
class TcpSession : public std::enable_shared_from_this<TcpSession> {
public:
    /**
     * @brief Создаёт сессию для принятого TCP-подключения.
     *
     * @param socket Сокет подключённого клиента.
     * @param block_size Размер статического блока команд.
     */
    TcpSession(boost::asio::ip::tcp::socket socket, std::size_t block_size);

    /// Завершает контекст обработки команд при уничтожении сессии.
    ~TcpSession();

    TcpSession(const TcpSession&) = delete;
    TcpSession& operator=(const TcpSession&) = delete;

    /// Запускает асинхронное чтение из сокета.
    void start();

private:
    /// Планирует следующую операцию чтения.
    void read();

    /// Завершает контекст обработки команд.
    void disconnect();

    /// Размер буфера одной асинхронной операции чтения.
    static constexpr std::size_t BUFFER_SIZE = 4096;

    /// Сокет подключённого клиента.
    boost::asio::ip::tcp::socket m_socket;

    /// Буфер очередной операции чтения.
    std::array<char, BUFFER_SIZE> m_buffer{};

    /// Контекст клиента в библиотеке async.
    async::handle_t m_handle;
};

#endif  // TCP_SESSION_H
