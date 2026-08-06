#ifndef ASYNC_H
#define ASYNC_H

/**
 * @file async.h
 * @brief Публичный интерфейс библиотеки async.
 */

#include <cstddef>

namespace async {

/** @addtogroup async_group
 *  @{
 */

/**
 * @brief Непрозрачный дескриптор подключения.
 *
 * Значение возвращается connect() и должно передаваться в receive() и
 * disconnect() без интерпретации вызывающим кодом.
 */
using handle_t = void*;

/**
 * @brief Создаёт новый контекст обработки команд.
 *
 * Контексты с одинаковым размером блока совместно накапливают статические
 * команды. Состояние динамических блоков остаётся независимым для каждого
 * контекста.
 *
 * @param bulk Размер статического блока команд.
 * @return Дескриптор созданного контекста.
 */
handle_t connect(std::size_t bulk);

/**
 * @brief Передаёт очередной фрагмент входных данных в контекст.
 *
 * @param handle Дескриптор контекста, полученный из connect().
 * @param data Указатель на начало входного буфера.
 * @param size Размер входного буфера в байтах.
 */
void receive(handle_t handle, const char* data, std::size_t size);

/**
 * @brief Завершает обработку и уничтожает контекст.
 *
 * @param handle Дескриптор контекста, полученный из connect().
 */
void disconnect(handle_t handle);

/** @} */

}  // namespace async

#endif  // ASYNC_H
