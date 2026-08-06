#include "bulk_format.h"

#include <sstream>

std::string format_bulk(const CommandBlock& block) {
    std::ostringstream output;
    output << "bulk: ";

    for (std::size_t i = 0; i < block.commands.size(); ++i) {
        if (i > 0) {
            output << ", ";
        }
        output << block.commands[i];
    }

    return output.str();
}
