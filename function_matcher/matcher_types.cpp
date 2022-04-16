#include "matcher_types.h"

typedef long off_t; // TODO: Find proper header
#include <rz_core.h>

namespace function_matcher {
    bool instruction_t::operator<(const instruction_t &other) const {
        if (op_len < other.op_len) {
            return true;
        } else if (op_len == other.op_len) {
            if (buffer < other.buffer) {
                return true;
            } else if (buffer == other.buffer) {
                if (mask < other.mask) {
                    return true;
                }
            }
        }
        return false;
    }

    void matcher_t::print(const std::string &name) const {
        rz_cons_printf("%s:\n  bytes: %llu\n  unique_instructions: %llu\n  total_instructions: %llu\n",
                       name.c_str(), bytes, instruction_counts.size(), instructions.size());
    }
}