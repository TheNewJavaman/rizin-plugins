#pragma once

typedef long off_t; // TODO: Find proper header
#include <rz_core.h>

namespace fnm {
    RzCmdStatus fnm_handler(RzCore *core, int argc, const char **argv);

    RzCmdStatus fnm_scan_all_handler(RzCore *core, int argc, const char **argv);

    RzCmdStatus fnm_scan_this_handler(RzCore *core, int argc, const char **argv);

    RzCmdStatus fnm_add_handler(RzCore *core, int argc, const char **argv);

    RzCmdStatus fnm_remove_handler(RzCore *core, int argc, const char **argv);

    RzCmdStatus fnm_remove_all_handler(RzCore *core, int argc, const char **argv);
}