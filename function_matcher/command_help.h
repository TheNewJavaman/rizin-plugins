#pragma once

typedef long off_t; // TODO: Find proper header
#include <rz_core.h>

namespace function_matcher {
    const RzCmdDescHelp fnm_group_help = {
            .summary = "Finds instructions similar to specified matchers",
            .options = "[/+-?]"
    };

    const RzCmdDescArg fnm_args[] = {
            nullptr
    };

    const RzCmdDescHelp fnm_help = {
            .summary = "Lists all matchers",
            .args = fnm_args
    };

    const RzCmdDescArg fnm_scan_args[] = {
            {
                    .name = "matcher",
                    .type = RZ_CMD_ARG_TYPE_STRING
            },
            nullptr
    };

    const RzCmdDescHelp fnm_scan_help = {
            .summary = "Scans for instructions similar to the specified matcher",
            .args = fnm_scan_args
    };

    const RzCmdDescArg fnm_add_args[] = {
            {
                    .name = "matcher",
                    .type = RZ_CMD_ARG_TYPE_STRING
            },
            {
                    .name = "function",
                    .type = RZ_CMD_ARG_TYPE_FCN
            },
            nullptr
    };

    const RzCmdDescHelp fnm_add_help = {
            .summary = "Creates a new matcher from the specified function",
            .args = fnm_add_args
    };

    const RzCmdDescHelp fnm_remove_group_help = {
            .summary = "Removes one or all matchers",
            .options = "[*?]"
    };

    const RzCmdDescArg fnm_remove_args[] = {
            {
                    .name = "matcher",
                    .type = RZ_CMD_ARG_TYPE_STRING
            },
            nullptr
    };

    const RzCmdDescHelp fnm_remove_help = {
            .summary = "Removes the specified matcher",
            .args = fnm_remove_args
    };

    const RzCmdDescArg fnm_remove_all_args[] = {
            nullptr
    };

    const RzCmdDescHelp fnm_remove_all_help = {
            .summary = "Removes all matchers",
            .args = fnm_remove_all_args
    };
}