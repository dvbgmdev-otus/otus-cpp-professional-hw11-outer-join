/**
 * @file async_test.cpp
 * @brief Интеграционные тесты публичного API библиотеки async.
 */

#include "async.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <limits.h>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

class AsyncApiTest : public ::testing::Test {
protected:
    void SetUp() override {
        // FileBulkWriter пишет файлы в текущий рабочий каталог.
        // Сохраняем исходный каталог, чтобы вернуть процесс обратно после теста.
        char current_dir[PATH_MAX];
        ASSERT_NE(nullptr, getcwd(current_dir, sizeof(current_dir)));
        m_previous_dir = current_dir;

        // Каждый тест выполняется в отдельном временном каталоге.
        // Это изолирует bulk*.log от других тестов и рабочего дерева проекта.
        std::string dir_template = "/tmp/async_api_test_XXXXXX";
        std::vector<char> dir_buffer(dir_template.begin(), dir_template.end());
        dir_buffer.push_back('\0');

        char* created_dir = mkdtemp(dir_buffer.data());
        ASSERT_NE(nullptr, created_dir);
        m_temp_dir = created_dir;

        // Переключаем текущий каталог, чтобы публичный async API писал
        // log-файлы именно в тестовую директорию.
        ASSERT_EQ(0, chdir(m_temp_dir.c_str()));
    }

    void TearDown() override {
        // Сначала удаляем файлы, пока текущий каталог всё ещё тестовый.
        remove_log_files();

        // Затем возвращаем исходный рабочий каталог процесса.
        if (!m_previous_dir.empty()) {
            const int chdir_result = chdir(m_previous_dir.c_str());
            EXPECT_EQ(0, chdir_result);
        }

        // Временная директория должна быть пустой после удаления log-файлов.
        if (!m_temp_dir.empty()) {
            rmdir(m_temp_dir.c_str());
        }
    }

    // Упрощает передачу строковых литералов в async::receive().
    void receive(async::handle_t handle, const char* data) {
        async::receive(handle, data, std::strlen(data));
    }

    std::vector<std::string> read_log_contents() const {
        // Имена файлов содержат timestamp и уникальный постфикс, поэтому
        // проверяем содержимое всех bulk*.log, а не конкретные имена.
        std::vector<std::string> contents;
        DIR* dir = opendir(".");
        EXPECT_NE(nullptr, dir);
        if (dir == nullptr) {
            return contents;
        }

        while (dirent* entry = readdir(dir)) {
            const std::string file_name = entry->d_name;
            if (!is_log_file(file_name)) {
                continue;
            }

            std::ifstream input(file_name);
            std::ostringstream content;
            content << input.rdbuf();
            contents.push_back(content.str());
        }

        closedir(dir);

        // Два файловых worker-а могут записать блоки в разном порядке.
        // Сортировка делает проверку независимой от планировщика потоков.
        std::sort(contents.begin(), contents.end());
        return contents;
    }

    void remove_log_files() const {
        // Удаляем только файлы, созданные FileBulkWriter в рамках теста.
        DIR* dir = opendir(".");
        if (dir == nullptr) {
            return;
        }

        while (dirent* entry = readdir(dir)) {
            const std::string file_name = entry->d_name;
            if (is_log_file(file_name)) {
                std::remove(file_name.c_str());
            }
        }

        closedir(dir);
    }

private:
    bool is_log_file(const std::string& file_name) const {
        // Файлы async имеют вид bulk<timestamp>_<postfix>.log.
        // Для теста достаточно проверить общий префикс и суффикс.
        const std::string prefix = "bulk";
        const std::string suffix = ".log";

        return file_name.size() > prefix.size() + suffix.size() &&
               file_name.compare(0, prefix.size(), prefix) == 0 &&
               file_name.compare(file_name.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    std::string m_previous_dir;
    std::string m_temp_dir;
};

#if (1)  // Part 1. Публичный API connect/receive/disconnect

// Test 1.1. Полный статический блок, переданный через публичный API, записывается в файл.
TEST_F(AsyncApiTest, Receive_WhenStaticBlockComplete_WritesLogFile) {
    async::handle_t handle = async::connect(2);
    receive(handle, "cmd1\ncmd2\n");
    async::disconnect(handle);

    EXPECT_EQ((std::vector<std::string>{ "bulk: cmd1, cmd2\n" }), read_log_contents());
}

// Test 1.2. Команда, переданная частями через несколько receive, склеивается.
TEST_F(AsyncApiTest, Receive_WhenLineSplitBetweenCalls_WritesMergedCommand) {
    async::handle_t handle = async::connect(1);
    receive(handle, "cm");
    receive(handle, "d1\n");
    async::disconnect(handle);

    EXPECT_EQ((std::vector<std::string>{ "bulk: cmd1\n" }), read_log_contents());
}

// Test 1.3. Несколько контекстов не смешивают незавершённые строки.
TEST_F(AsyncApiTest, Receive_WhenSeveralContextsUsed_KeepsPendingLinesSeparate) {
    async::handle_t first_handle = async::connect(1);
    async::handle_t second_handle = async::connect(1);

    receive(first_handle, "cm");
    receive(second_handle, "ab");
    receive(first_handle, "d1\n");
    receive(second_handle, "c\n");

    async::disconnect(first_handle);
    async::disconnect(second_handle);

    EXPECT_EQ((std::vector<std::string>{ "bulk: abc\n", "bulk: cmd1\n" }), read_log_contents());
}

// Test 1.4. Некорректные аргументы публичного API безопасно игнорируются.
TEST_F(AsyncApiTest, Receive_WhenArgumentsInvalid_DoesNotCreateLogFiles) {
    async::receive(nullptr, "cmd1\n", 5);
    async::disconnect(nullptr);

    async::handle_t handle = async::connect(1);
    async::receive(handle, nullptr, 5);
    async::receive(handle, "cmd1\n", 0);
    async::disconnect(handle);

    EXPECT_TRUE(read_log_contents().empty());
}

#endif  // Part 1. Публичный API connect/receive/disconnect

#if (1)  // Part 2. Совместная обработка нескольких подключений

// Test 2.1. Статические команды разных контекстов формируют общий блок.
TEST_F(AsyncApiTest, Receive_WhenStaticCommandsComeFromSeveralContexts_MergesCommands) {
    async::handle_t first_handle = async::connect(3);
    async::handle_t second_handle = async::connect(3);

    receive(first_handle, "cmd1\n");
    receive(second_handle, "cmd2\n");
    receive(first_handle, "cmd3\n");

    async::disconnect(first_handle);
    async::disconnect(second_handle);

    EXPECT_EQ((std::vector<std::string>{ "bulk: cmd1, cmd2, cmd3\n" }),
              read_log_contents());
}

// Test 2.2. Динамические блоки разных контекстов не смешиваются.
TEST_F(AsyncApiTest, Receive_WhenDynamicBlocksComeFromSeveralContexts_KeepsBlocksSeparate) {
    async::handle_t first_handle = async::connect(3);
    async::handle_t second_handle = async::connect(3);

    receive(first_handle, "{\ncmd1\n");
    receive(second_handle, "{\ncmd2\n");
    receive(first_handle, "}\n");
    receive(second_handle, "}\n");

    async::disconnect(first_handle);
    async::disconnect(second_handle);

    EXPECT_EQ((std::vector<std::string>{ "bulk: cmd1\n", "bulk: cmd2\n" }),
              read_log_contents());
}

// Test 2.3. Отключение одного контекста не завершает общий статический блок.
TEST_F(AsyncApiTest, Disconnect_WhenOtherContextIsActive_DoesNotFlushStaticBlock) {
    async::handle_t first_handle = async::connect(3);
    async::handle_t second_handle = async::connect(3);

    receive(first_handle, "cmd1\n");
    receive(second_handle, "cmd2\n");

    async::disconnect(first_handle);
    EXPECT_TRUE(read_log_contents().empty());

    async::disconnect(second_handle);
    EXPECT_EQ((std::vector<std::string>{ "bulk: cmd1, cmd2\n" }), read_log_contents());
}

// Test 2.4. Начало динамического блока завершает накопленный общий статический блок.
TEST_F(AsyncApiTest, Receive_WhenDynamicBlockStarts_FlushesSharedStaticBlock) {
    async::handle_t first_handle = async::connect(3);
    async::handle_t second_handle = async::connect(3);

    receive(first_handle, "cmd1\n");
    receive(second_handle, "{\ncmd2\n}\n");

    async::disconnect(first_handle);
    async::disconnect(second_handle);

    EXPECT_EQ((std::vector<std::string>{ "bulk: cmd1\n", "bulk: cmd2\n" }),
              read_log_contents());
}

#endif  // Part 2. Совместная обработка нескольких подключений
