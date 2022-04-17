#include "command_handlers.h"

#include "matcher_types.h"

namespace fnm {
    constexpr uint32_t MINIMUM_CHAIN_LENGTH = 5;
    constexpr uint32_t MAXIMUM_CHAIN_GAP = 20;

    std::map<std::string, matcher_t> matchers;

    matcher_t create_matcher(RzCore *core, const buffer_t &buffer, size_t base, size_t start, int32_t bytes) {
        matcher_t matcher = {
                .bytes = 0
        };
        for (int32_t i = 0, op_len; i < bytes; i += op_len) {
            RzAnalysisOp op;
            op_len = core->analysis->cur->op(core->analysis, &op, start + i, buffer.data() + i,
                                             bytes - i, RZ_ANALYSIS_OP_MASK_BASIC);
            if (i + op_len > bytes) {
                break;
            }
            auto mask = rz_analysis_mask(core->analysis, op_len, buffer.data() + i, start + i);
            instruction_t instruction = {
                    .addr = base + start + i,
                    .bytes = op_len,
                    .buffer = buffer_t(buffer.data() + i, buffer.data() + i + op_len),
                    .mask = buffer_t(mask, mask + op_len)
            };
            matcher.bytes += op_len;
            matcher.instructions.push_back(instruction);
            matcher.instruction_counts[instruction]++;
        }
        return matcher;
    }

    /*
    match_t create_match(const matcher_t &matcher1, const matcher_t &matcher2, std::vector<chain_t>) {
        match_t match;
        std::vector<chain_t> active_chains;
        auto instruction_count = std::min(matcher1.instructions.size(), matcher2.instructions.size());
        for (size_t i = 0; i < instruction_count; i++) {
            auto &instruction1 = matcher1.instructions.at(i);
            auto &instruction2 = matcher2.instructions.at(i);
            if (instruction1 == instruction2) {
                match.common_instructions_count++;

                for (auto &active_chain: active_chains) {
                    active_chain.instruction_end = i;
                }
            }
        }
        for (auto &chain: active_chains) {
            chain.does_overflow = true;
            chain.instruction_end = instruction_count - 1;
        }
        return match;
    }
    */

    RzCmdStatus fnm_handler(RzCore *core, int argc, const char **argv) {
        if (argc == 1) {
            for (const auto &[name, matcher]: matchers) {
                matcher.print(name);
            }
        } else {
            auto matcher_iter = matchers.find(argv[1]);
            if (matcher_iter == matchers.end()) {
                eprintf("Error: Couldn't find matcher with name %s\n", argv[1]);
                return RZ_CMD_STATUS_ERROR;
            } else {
                matcher_iter->second.print(argv[1]);
            }
        }
        return RZ_CMD_STATUS_OK;
    }

    RzCmdStatus fnm_scan_all_handler(RzCore *core, int argc, const char **argv) {
        auto maps = rz_io_maps(core->io);
        void **it;
        rz_pvector_foreach(maps, it)
        {
            auto map = reinterpret_cast<RzIOMap *>(it);
            auto from = map->itv.addr;
            auto to = map->itv.addr + map->itv.size;
            rz_cons_printf("Scanning 0x%08llx - 0x%08llx...\n", from, to);

        }
        return RZ_CMD_STATUS_OK;

        /*
        RzListIter *iter;
        void *map;
        rz_list_foreach(){

                for (auto at = from; at < to; at += core->blocksize) {
                    if (!rz_io_is_valid_offset(core->io, at, 0)) break;
                    auto len = static_cast<int32_t>(std::min(static_cast<uint64_t>(core->blocksize), to - at));
                    std::vector<uint8_t> buffer(core->blocksize);
                    rz_io_read_at(core->io, at, buffer.data(), len);
                }
        }
        rz_list_free(io_bounds);
        */
    }

    bool in_chain_range(size_t i, size_t j, const chain_common_t &common_chain) {
        return common_chain.start_this <= i &&
               i < common_chain.start_this + common_chain.len &&
               j - common_chain.start_orig == i - common_chain.start_this;
    }

    bool exceeds_gap_range(const chain_common_t &last_chain, const chain_common_t &next_chain) {
        return last_chain.start_this + last_chain.len + MAXIMUM_CHAIN_GAP < next_chain.start_this;
    }

    bool in_gap_range(const chain_common_t &last_chain, const chain_common_t &next_chain) {
        return last_chain.start_this + last_chain.len <= next_chain.start_this;
    }

    RzCmdStatus fnm_scan_this_handler(RzCore *core, int argc, const char **argv) {
        auto orig_matcher_iter = matchers.find(argv[1]);
        if (orig_matcher_iter == matchers.end()) {
            eprintf("Error: Couldn't find matcher with name %s\n", argv[1]);
            return RZ_CMD_STATUS_ERROR;
        }
        auto orig_matcher = orig_matcher_iter->second;
        auto len = orig_matcher.bytes;
        buffer_t buffer(len + 32); // Pad buffer in case instruction overflows
        rz_io_read_at_mapped(core->io, core->offset, buffer.data(), len + 32);
        auto this_matcher = create_matcher(core, buffer, core->offset, 0, len);
        std::vector<chain_common_t> common_chains;
        for (size_t i = 0; i < this_matcher.instructions.size() - MINIMUM_CHAIN_LENGTH; i++) {
            auto this_instruction = this_matcher.instructions.at(i);
            for (size_t j = 0; j < orig_matcher.instructions.size(); j++) {
                auto orig_instruction = orig_matcher.instructions.at(j);
                if (this_instruction == orig_instruction) {
                    bool should_start_chain = true;
                    for (const auto &common_chain: common_chains) {
                        if (in_chain_range(i, j, common_chain)) {
                            should_start_chain = false;
                            break;
                        }
                    }
                    if (should_start_chain) {
                        chain_common_t chain = {
                                .addr = this_instruction.addr,
                                .bytes = orig_instruction.bytes,
                                .start_orig = j,
                                .start_this = i
                        };
                        size_t k = 1;
                        for (; i + k < this_matcher.instructions.size() &&
                               j + k < orig_matcher.instructions.size(); k++) {
                            auto this_next_instruction = this_matcher.instructions.at(i + k);
                            auto orig_next_instruction = orig_matcher.instructions.at(j + k);
                            if (this_next_instruction == orig_next_instruction) {
                                chain.bytes += orig_next_instruction.bytes;
                            } else {
                                break;
                            }
                        }
                        chain.len = static_cast<int32_t>(k);
                        if (chain.len >= MINIMUM_CHAIN_LENGTH) {
                            common_chains.push_back(chain);
                        }
                    }
                }
            }
        }
        std::vector<match_t> matches;
        for (size_t i = 0; i < common_chains.size(); i++) {
            auto this_chain = common_chains.at(i);
            bool should_start_match = true;
            for (const auto &match: matches) {
                if (match.has_common_chain(this_chain)) {
                    should_start_match = false;
                    break;
                }
            }
            if (should_start_match) {
                instruction_count_t instruction_counts;
                match_t match = {
                        .addr = this_chain.addr,
                        .bytes = this_chain.bytes,
                        .len = this_chain.len,
                        .common_chains = {this_chain}
                };
                for (size_t j = 1; i + j < common_chains.size(); j++) {
                    auto last_chain = common_chains.at(i + j - 1);
                    auto next_chain = common_chains.at(i + j);
                    if (exceeds_gap_range(last_chain, next_chain)) {
                        break;
                    } else if (in_gap_range(last_chain, next_chain)) {
                        match.bytes += next_chain.bytes;
                        match.len += next_chain.len;
                        match.common_chains.push_back(next_chain);
                        auto gap = static_cast<int32_t>(next_chain.start_this - last_chain.start_this + last_chain.len);
                        if (gap > 0) {
                            auto bytes = static_cast<int32_t>(next_chain.addr - last_chain.addr + last_chain.bytes);
                            chain_excess_t excess_chain = {
                                    .addr = last_chain.addr + last_chain.bytes,
                                    .bytes = bytes,
                                    .start_this = last_chain.start_this + last_chain.len,
                                    .len = gap
                            };
                            match.excess_chains.push_back(excess_chain);
                            match.bytes += bytes;
                            match.len += gap;
                        }
                    }
                }
                match.calc_coverage(orig_matcher);
                matches.push_back(match);
                match.print();
            }
        }
        return RZ_CMD_STATUS_OK;
    }

    RzCmdStatus fnm_add_handler(RzCore *core, int argc, const char **argv) {
        auto orig_offset = core->offset;
        auto func = rz_analysis_get_function_byname(core->analysis, argv[2]);
        rz_core_seek(core, orig_offset, true);
        if (func == nullptr) {
            eprintf("Error: Couldn't find function with name %s\n", argv[2]);
            return RZ_CMD_STATUS_ERROR;
        }
        auto len = static_cast<int32_t>(func->meta._max - func->addr);
        buffer_t buffer(len);
        rz_io_read_at_mapped(core->io, func->addr, buffer.data(), len);
        auto matcher = create_matcher(core, buffer, func->addr, 0, len);
        matchers.insert({argv[1], matcher});
        matcher.print(argv[1]);
        return RZ_CMD_STATUS_OK;
    }

    RzCmdStatus fnm_remove_handler(RzCore *core, int argc, const char **argv) {
        if (matchers.erase(argv[1]) == 0) {
            eprintf("Error: Couldn't find matcher with name %s\n", argv[1]);
            return RZ_CMD_STATUS_ERROR;
        }
        return RZ_CMD_STATUS_OK;
    }

    RzCmdStatus fnm_remove_all_handler(RzCore *core, int argc, const char **argv) {
        matchers.clear();
        return RZ_CMD_STATUS_OK;
    }
}