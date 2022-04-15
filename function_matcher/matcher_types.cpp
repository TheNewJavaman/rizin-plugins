#include "matcher_types.h"

typedef long off_t; // TODO: Find proper header
#include <rz_core.h>

namespace function_matcher {
    bool instruction_t::operator<(const instruction_t &other) const {
        return op < other.op || (op == other.op && payload < other.payload);
    }

    void matcher_t::print(const std::string &name) const {
        rz_cons_printf("%s:\n  unique_instructions: %llu\n  total_instructions: %llu\n",
                       name.c_str(), instruction_counts.size(), instructions.size());
    }
}