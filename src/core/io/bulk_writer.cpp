#include "bulk_writer.h"

#include <fstream>
#include <string>

#include "bulk_format.h"

namespace {

std::string make_log_file_name(const CommandBlock& block) {
    return "bulk" + std::to_string(block.timestamp) + ".log";
}

std::string make_log_file_name(const CommandBlock& block, std::size_t postfix) {
    return "bulk" + std::to_string(block.timestamp) + "_" + std::to_string(postfix) + ".log";
}

}  // namespace

ConsoleBulkWriter::ConsoleBulkWriter(std::ostream& output) : m_output(output) {}

void ConsoleBulkWriter::write(const CommandBlock& block) { m_output << format_bulk(block) << '\n'; }

void FileBulkWriter::write(const CommandBlock& block) {
    std::ofstream output(make_log_file_name(block));
    output << format_bulk(block) << '\n';
}

void FileBulkWriter::write(const CommandBlock& block, std::size_t postfix) {
    std::ofstream output(make_log_file_name(block, postfix));
    output << format_bulk(block) << '\n';
}
