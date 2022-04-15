#pragma once

typedef long off_t; // TODO: Find proper header

#include <map>
#include <rz_core.h>
#include <string>
#include <vector>

namespace function_matcher {
    struct matcher_t {
        std::map<std::string, uint32_t> instruction_counts;

        std::vector<std::string> instructions;
    };

    RzCmdStatus fnm_handler(RzCore *core, int argc, const char **argv);

    RzCmdStatus fnm_add_handler(RzCore *core, int argc, const char **argv);

    RzCmdStatus fnm_remove_handler(RzCore *core, int argc, const char **argv);
}