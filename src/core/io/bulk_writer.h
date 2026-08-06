#ifndef BULK_WRITER_H
#define BULK_WRITER_H

/**
 * @file bulk_writer.h
 * @brief Вывод готовых блоков команд.
 */

#include <cstddef>
#include <ostream>

#include "command_block.h"

/**
 * @brief Пишет готовые блоки команд в выходной поток.
 * @ingroup output_group
 */
class ConsoleBulkWriter {
public:
    /**
     * @brief Создаёт writer для заданного потока.
     *
     * @param output Поток, в который будут записываться блоки.
     */
    explicit ConsoleBulkWriter(std::ostream& output);

    /**
     * @brief Записывает блок команд в поток.
     *
     * @param block Готовый блок команд.
     */
    void write(const CommandBlock& block);

private:
    /// Поток вывода для готовых блоков.
    std::ostream& m_output;
};

/**
 * @brief Пишет готовые блоки команд в log-файлы.
 * @ingroup output_group
 */
class FileBulkWriter {
public:
    /**
     * @brief Записывает блок команд в файл.
     *
     * Имя файла формируется как `bulk<timestamp>.log`.
     *
     * @param block Готовый блок команд.
     */
    void write(const CommandBlock& block);

    /**
     * @brief Записывает блок команд в файл с дополнительным постфиксом.
     *
     * Имя файла формируется как `bulk<timestamp>_<postfix>.log`.
     *
     * @param block Готовый блок команд.
     * @param postfix Уникальный постфикс имени файла.
     */
    void write(const CommandBlock& block, std::size_t postfix);
};

#endif  // BULK_WRITER_H
