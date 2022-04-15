typedef long off_t; // TODO: Find proper header

#include <rz_core.h>

namespace function_matcher {
    const RzCmdDescArg fnm_args[] = {
            {
                    .name = "matcher_name",
                    .type = RZ_CMD_ARG_TYPE_STRING
            },
            nullptr
    };

    const RzCmdDescHelp fnm_help = {
            .summary = "Finds instructions similar to the specified matcher",
            .args = fnm_args
    };

    const RzCmdDescHelp fnm_group_help = {
            .summary = "Finds instructions similar to a specified matcher",
            .options = "[+-?]"
    };

    const RzCmdDescArg fnm_add_args[] = {
            {
                    .name = "function_name",
                    .type = RZ_CMD_ARG_TYPE_FCN
            },
            {
                    .name = "matcher_name",
                    .type = RZ_CMD_ARG_TYPE_STRING
            },
            nullptr
    };

    const RzCmdDescHelp fnm_add_help = {
            .summary = "Adds the current function's instructions as a new matcher",
            .args = fnm_add_args
    };

    const RzCmdDescArg fnm_remove_args[] = {
            {
                    .name = "matcher_name",
                    .type = RZ_CMD_ARG_TYPE_STRING
            },
            nullptr
    };

    const RzCmdDescHelp fnm_remove_help = {
            .summary = "Removes the specified matcher",
            .args = fnm_remove_args
    };
}