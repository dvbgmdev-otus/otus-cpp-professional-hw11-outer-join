#include <csignal>
#include <exception>
#include <iostream>

#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/system/error_code.hpp>

#include "cli_args.h"
#include "command_processor.h"
#include "database.h"
#include "tcp_server.h"

namespace {

constexpr int EXIT_SUCCESS_CODE = 0;
constexpr int EXIT_ERROR_CODE = 1;

}  // namespace

int main(int argc, const char* const argv[]) {
    try {
        const CliArgsParseResult cli_args = parse_cli_args(argc, argv);
        if (!cli_args.success) {
            std::cerr << cli_args.error << '\n';
            return EXIT_ERROR_CODE;
        }

        boost::asio::io_context io_context;
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait(
            [&io_context](const boost::system::error_code&, int) { io_context.stop(); });

        Database database;
        CommandProcessor command_processor(database);
        TcpServer server(io_context, cli_args.port, command_processor);
        server.start();
        io_context.run();

        return EXIT_SUCCESS_CODE;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return EXIT_ERROR_CODE;
    }
}
