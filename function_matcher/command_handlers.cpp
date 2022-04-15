#include "command_handlers.h"

namespace function_matcher {
    std::map<std::string, matcher_t> matchers;

    RzCmdStatus fnm_handler(RzCore *core, int argc, const char **argv) {
        return RZ_CMD_STATUS_OK;
    }

    RzCmdStatus fnm_add_handler(RzCore *core, int argc, const char **argv) {
        return RZ_CMD_STATUS_OK;
    }

    RzCmdStatus fnm_remove_handler(RzCore *core, int argc, const char **argv) {
        return RZ_CMD_STATUS_OK;
    }
}