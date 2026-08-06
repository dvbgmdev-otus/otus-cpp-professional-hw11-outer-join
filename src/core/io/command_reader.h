#ifndef COMMAND_READER_H
#define COMMAND_READER_H

/**
 * @file command_reader.h
 * @brief Построчное чтение команд из входного потока.
 */

#include <ctime>
#include <functional>
#include <istream>
#include <string>

/**
 * @brief Читает команды из входного потока.
 * @ingroup input_group
 *
 * CommandReader не содержит правил пакетирования и не знает о формате вывода.
 * Он преобразует входной поток в события: получена строка команды или достигнут
 * конец ввода.
 */
class CommandReader {
public:
    /**
     * @brief Обработчик прочитанной команды.
     *
     * Получает строку команды и время её получения.
     */
    using CommandHandler = std::function<void(const std::string&, std::time_t)>;

    /// Обработчик окончания входного потока.
    using FinishHandler = std::function<void()>;

    /// Источник времени для фиксации момента получения команды.
    using Clock = std::function<std::time_t()>;

    /**
     * @brief Создаёт reader для заданного входного потока.
     *
     * Если clock не задан, используется системное время.
     *
     * @param input Входной поток с командами.
     * @param command_handler Обработчик каждой прочитанной строки.
     * @param finish_handler Обработчик окончания входного потока.
     * @param clock Источник времени для команд.
     */
    CommandReader(std::istream& input,
                  CommandHandler command_handler,
                  FinishHandler finish_handler,
                  Clock clock = Clock{});

    /**
     * @brief Читает весь входной поток.
     *
     * Для каждой строки вызывает CommandHandler. После достижения EOF вызывает
     * FinishHandler ровно один раз.
     */
    void read_all();

private:
    /// Входной поток, из которого читаются команды.
    std::istream& m_input;

    /// Callback для передачи каждой прочитанной команды.
    CommandHandler m_command_handler;

    /// Callback для уведомления об окончании входного потока.
    FinishHandler m_finish_handler;

    /// Источник времени получения команд.
    Clock m_clock;
};

#endif  // COMMAND_READER_H
