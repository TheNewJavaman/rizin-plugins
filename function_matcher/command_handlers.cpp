#include "command_handlers.h"

#include "matcher_types.h"

namespace function_matcher {
    std::map<std::string, matcher_t> matchers;

    RzCmdStatus fnm_handler(RzCore *core, int argc, const char **argv) {
        for (const auto &[name, matcher]: matchers) {
            matcher.print(name);
        }
        return RZ_CMD_STATUS_OK;
    }

    RzCmdStatus fnm_scan_handler(RzCore *core, int argc, const char **argv) {
        return RZ_CMD_STATUS_OK;
    }

    RzCmdStatus fnm_add_handler(RzCore *core, int argc, const char **argv) {
        auto original_offset = core->offset;
        auto func = rz_analysis_get_function_byname(core->analysis, argv[2]);
        if (func == nullptr) {
            eprintf("fnm+: Couldn't find function with name %s\n", argv[2]);
            return RZ_CMD_STATUS_INVALID;
        }
        auto start = func->addr;
        auto end = rz_analysis_function_max_addr(func);
        matcher_t matcher;
        if (end > start) {
            rz_core_seek(core, start, true);
            auto num_bytes = end - start;
            matcher.bytes = num_bytes;
            rz_core_block_size(core, num_bytes);
            rz_io_read_at(core->io, core->offset, core->block, static_cast<int32_t>(num_bytes));
            for (size_t i = 0, len = 0; i < num_bytes; i += len) {
                RzAnalysisOp op;
                len = core->analysis->cur->op(core->analysis, &op, start + i, core->block + i,
                                              static_cast<int32_t>(num_bytes - i), RZ_ANALYSIS_OP_MASK_BASIC);
                auto mask = rz_analysis_mask(core->analysis, len, core->block + i, start + i);
                instruction_t instruction = {
                        .op_len = len,
                        .buffer = buffer_t(core->block + i, core->block + i + len),
                        .mask = buffer_t(mask, mask + len)
                };
                matcher.instructions.push_back(instruction);
                matcher.instruction_counts[instruction]++;
            }
        }
        matchers.insert({argv[1], matcher});
        matcher.print(argv[1]);
        rz_core_seek(core, original_offset, true);
        return RZ_CMD_STATUS_OK;
    }

    RzCmdStatus fnm_remove_handler(RzCore *core, int argc, const char **argv) {
        matchers.erase(argv[1]);
        return RZ_CMD_STATUS_OK;
    }

    RzCmdStatus fnm_remove_all_handler(RzCore *core, int argc, const char **argv) {
        matchers.clear();
        return RZ_CMD_STATUS_OK;
    }
}