#include "command_handlers.h"

#include "matcher_types.h"

namespace fnm {
    constexpr uint32_t MINIMUM_CHAIN_LENGTH = 5;
    constexpr uint32_t MAXIMUM_CHAIN_GAP = 20;

    std::map<std::string, matcher_t> matchers;

    void extend_matcher(RzCore *core, matcher_t &matcher, const buffer_t &buffer, size_t addr, uint32_t bytes) {
        RzAnalysisOp op;
        uint32_t op_len;
        for (uint32_t i = 0; i < bytes; i += op_len) {
            op_len = core->analysis->cur->op(core->analysis, &op, i, buffer.data() + i,
                                             static_cast<int32_t>(bytes - i), RZ_ANALYSIS_OP_MASK_BASIC);
            if (i + op_len > bytes) {
                break;
            }
            auto mask = rz_analysis_mask(core->analysis, op_len, buffer.data() + i, i);
            instruction_t instruction = {
                    .addr = addr + i,
                    .bytes = op_len,
                    .buffer = buffer_t(buffer.data() + i, buffer.data() + i + op_len),
                    .mask = buffer_t(mask, mask + op_len)
            };
            matcher.bytes += op_len;
            matcher.instructions.push_back(instruction);
            matcher.instruction_counts[instruction]++;
        }
    }

    bool in_chain_range(size_t i, size_t j, const chain_common_t &common_chain) {
        return common_chain.start_this <= i &&
               i < common_chain.start_this + common_chain.len &&
               j - common_chain.start_orig == i - common_chain.start_this;
    }

    bool after_gap_range(const chain_common_t &last_chain, const chain_common_t &next_chain) {
        return last_chain.start_this + last_chain.len + MAXIMUM_CHAIN_GAP < next_chain.start_this;
    }

    bool before_gap_range(const chain_common_t &last_chain, const chain_common_t &next_chain) {
        return next_chain.start_this < last_chain.start_this + last_chain.len;
    }

    void scan_matchers(const matcher_t &orig_matcher, const matcher_t &this_matcher) {
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
                        uint32_t k = 1;
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
                        chain.len = k;
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
                    if (after_gap_range(last_chain, next_chain)) {
                        break;
                    } else if (!before_gap_range(last_chain, next_chain)) {
                        match.bytes += next_chain.bytes;
                        match.len += next_chain.len;
                        match.common_chains.push_back(next_chain);
                        auto gap = static_cast<uint32_t>(next_chain.start_this - last_chain.start_this + last_chain.len);
                        if (gap > 0) {
                            auto bytes = static_cast<uint32_t>(next_chain.addr - last_chain.addr + last_chain.bytes);
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
    }

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
        auto orig_matcher_iter = matchers.find(argv[1]);
        if (orig_matcher_iter == matchers.end()) {
            eprintf("Error: Couldn't find matcher with name %s\n", argv[1]);
            return RZ_CMD_STATUS_ERROR;
        }
        auto orig_matcher = orig_matcher_iter->second;
        auto boundaries = rz_core_get_boundaries_prot(core, -1, "section.text", "search");
        if (boundaries == nullptr) {
            eprintf("Error: Couldn't get boundaries");
            return RZ_CMD_STATUS_ERROR;
        }
        buffer_t buffer(core->blocksize + 32);
        matcher_t this_matcher = {.bytes = 0};
        void *map;
        for (RzListIter *iter = boundaries->head; iter && (map = iter->data, 1); iter = iter->n) {
            auto io_map = static_cast<RzIOMap *>(map);
            auto start = rz_itv_begin(io_map->itv);
            auto end = rz_itv_end(io_map->itv);
            rz_cons_printf("Searching 0x%08" PFMT64x "-0x%08" PFMT64x "...\n", start, end);
            for (uint64_t at = start; at < end; at += core->blocksize) {
                auto min_len = std::min(core->blocksize + 32, static_cast<uint32_t>(end - at));
                if (!rz_io_is_valid_offset(core->io, at, 0)) {
                    break;
                }
                rz_io_read_at(core->io, at, buffer.data(), static_cast<int32_t>(min_len));
                extend_matcher(core, this_matcher, buffer, at, core->blocksize);
            }
        }
        rz_list_free(boundaries);
        scan_matchers(orig_matcher, this_matcher);
        return RZ_CMD_STATUS_OK;
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
        rz_io_read_at_mapped(core->io, core->offset, buffer.data(), static_cast<int32_t>(len) + 32);
        matcher_t this_matcher = {.bytes = 0};
        extend_matcher(core, this_matcher, buffer, core->offset, len);
        scan_matchers(orig_matcher, this_matcher);
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
        auto len = static_cast<uint32_t>(func->meta._max - func->addr);
        buffer_t buffer(len);
        rz_io_read_at_mapped(core->io, func->addr, buffer.data(), static_cast<int32_t>(len));
        matcher_t matcher = {.bytes = 0};
        extend_matcher(core, matcher, buffer, func->addr, len);
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