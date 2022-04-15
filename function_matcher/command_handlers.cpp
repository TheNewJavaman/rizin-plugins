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
        auto func = rz_analysis_get_function_byname(core->analysis, argv[1]);
        if (func == nullptr) {
            eprintf("fnm+: Couldn't find function with name %s\n", argv[1]);
            return RZ_CMD_STATUS_INVALID;
        }
        auto start = func->addr;
        auto end = rz_analysis_function_max_addr(func);
        matcher_t matcher;
        if (end > start) {
            rz_core_seek(core, start, true);
            auto num_bytes = static_cast<int32_t>(end - start);
            auto original_offset = core->offset;
            rz_core_block_size(core, num_bytes);
            rz_io_read_at(core->io, core->offset, core->block, num_bytes);
            byte_t *buffer = core->block;
            for (int32_t i = 0; i < num_bytes;) {
                RzAsmOp op;
                i += rz_asm_disassemble(core->rasm, &op, buffer + i, num_bytes - i);
                instruction_t instruction = {
                        .op = buffer_t(op.buf.buf, op.buf.buf + op.size - op.payload),
                        .payload = buffer_t(op.buf.buf + op.size - op.payload, op.buf.buf + op.size)
                };
                matcher.instructions.push_back(instruction);
                matcher.instruction_counts[instruction]++;
            }
            rz_core_seek(core, original_offset, true);
        }
        matchers.insert({argv[2], matcher});
        matcher.print(argv[2]);
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