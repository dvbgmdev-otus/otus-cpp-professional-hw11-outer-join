#ifndef TCP_SESSION_H
#define TCP_SESSION_H

/**
 * @file tcp_session.h
 * @brief Асинхронная TCP-сессия одного клиента.
 */

#include <array>
#include <cstddef>
#include <deque>
#include <memory>
#include <string>

#include <boost/asio/ip/tcp.hpp>

class CommandProcessor;

/**
 * @brief Читает и обрабатывает команды одного TCP-клиента.
 * @ingroup main_group
 */
class TcpSession : public std::enable_shared_from_this<TcpSession> {
public:
    /**
     * @brief Создаёт сессию для принятого TCP-подключения.
     *
     * @param socket Сокет подключённого клиента.
     * @param command_processor Общий обработчик команд сервера.
     */
    TcpSession(boost::asio::ip::tcp::socket socket, CommandProcessor& command_processor);

    TcpSession(const TcpSession&) = delete;
    TcpSession& operator=(const TcpSession&) = delete;

    /// Запускает асинхронное чтение из сокета.
    void start();

private:
    /// Планирует следующую операцию чтения.
    void read();

    /// Извлекает завершённые команды из входного буфера.
    void process_commands();

    /// Планирует отправку первого ответа из очереди.
    void write();

    /// Размер буфера одной асинхронной операции чтения.
    static constexpr std::size_t BUFFER_SIZE = 4096;

    /// Сокет подключённого клиента.
    boost::asio::ip::tcp::socket m_socket;

    /// Буфер очередной операции чтения.
    std::array<char, BUFFER_SIZE> m_buffer{};

    /// Общий обработчик команд сервера.
    CommandProcessor& m_CommandProcessor;

    /// Накопленные входные данные, включая незавершённую команду.
    std::string m_Input;

    /// Ответы, ожидающие отправки клиенту.
    std::deque<std::string> m_Responses;
};

#endif  // TCP_SESSION_H
