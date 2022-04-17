#include "matcher_types.h"

#include <algorithm>

typedef long off_t; // TODO: Find proper header
#include <rz_core.h>

#define ASSERT_LESS(a, b) \
    if ((a) < (b)) {      \
        return true;      \
    }
#define ASSERT_LESS_CHAIN(a, b) \
    ASSERT_LESS(a, b)           \
    else if ((a) == (b))
#define ASSERT_EQUAL(a, b) \
    if ((a) != (b)) {      \
        return false;      \
    }

namespace fnm {
    bool instruction_t::operator<(const instruction_t &other) const {
        ASSERT_LESS_CHAIN(bytes, other.bytes) {
            ASSERT_LESS_CHAIN(buffer, other.buffer) {
                ASSERT_LESS(mask, other.mask)
            }
        }
        return false;
    }

    bool instruction_t::operator==(const instruction_t &other) const {
        ASSERT_EQUAL(bytes, other.bytes)
        ASSERT_EQUAL(mask, other.mask)
        ASSERT_EQUAL(mask, other.mask)
        ASSERT_EQUAL(buffer.size(), other.buffer.size())
        for (size_t i = 0; i < buffer.size(); i++) {
            if (mask.at(i) == 0xFF) {
                ASSERT_EQUAL(buffer.at(i), other.buffer.at(i))
            }
        }
        return true;
    }

    void matcher_t::print(const std::string &name) const {
        rz_cons_printf("%s:\n  bytes: %d\n  instructions: %llu\n", name.c_str(), bytes, instructions.size());
    }

    bool chain_common_t::operator==(const chain_common_t &other) const {
        ASSERT_EQUAL(addr, other.addr)
        ASSERT_EQUAL(bytes, other.bytes)
        ASSERT_EQUAL(start_orig, other.start_orig)
        ASSERT_EQUAL(start_this, other.start_this)
        ASSERT_EQUAL(len, other.len)
        return true;
    }

    void chain_common_t::print(size_t i) const {
        rz_cons_printf("    %zu:\n      address_range: 0x%08" PFMT64x "-0x%08" PFMT64x "\n      matcher_instructions: "
                       "%zu-%zu\n      scanned_instructions: %zu-%zu\n", i, addr, addr + bytes, start_orig,
                       start_orig + len - 1, start_this, start_this + len - 1);
    }

    void chain_missing_t::print(size_t i) const {
        rz_cons_printf("    %zu:\n      matcher_instructions: %zu-%zu\n", i, start_orig, start_orig + len);
    }

    void chain_excess_t::print(size_t i) const {
        rz_cons_printf("    %zu:\n      address_range: 0x%08" PFMT64x "-0x%08" PFMT64x "\n      scanned_instructions: "
                       "%zu-%zu\n", i, addr, addr + bytes, start_this, start_this + len - 1);
    }

    bool match_t::has_common_chain(const chain_common_t &chain) const {
        return std::any_of(common_chains.begin(), common_chains.end(), [chain](const auto &match_chain) {
            return match_chain == chain;
        });
    }

    void match_t::calc_coverage(const matcher_t &matcher) {
        instruction_count_t instruction_counts;
        for (const auto &chain: common_chains) {
            for (size_t i = 0; i < chain.len; i++) {
                instruction_counts[matcher.instructions.at(chain.start_orig + i)]++;
            }
        }
        size_t common_bytes = 0;
        size_t common_instructions = 0;
        for (const auto &[instruction, orig_count]: matcher.instruction_counts) {
            auto this_count = instruction_counts[instruction];
            auto common_count = std::min(orig_count, this_count);
            common_bytes += common_count * instruction.bytes;
            common_instructions += common_count;
        }
        byte_coverage = static_cast<float>(common_bytes) / static_cast<float>(matcher.bytes);
        instruction_coverage = static_cast<float>(common_instructions) /
                               static_cast<float>(matcher.instructions.size());
    }

    void match_t::print() const {
        rz_cons_printf("0x%08" PFMT64x ":\n  bytes: %d\n  instructions: %d\n  byte_coverage: %.5f\n  "
                       "instruction_coverage: %.5f\n  common_chains:\n", addr, bytes, len, byte_coverage,
                       instruction_coverage);
        for (size_t i = 0; i < common_chains.size(); i++) {
            common_chains.at(i).print(i);
        }
        rz_cons_printf("  missing_chains:\n");
        for (size_t i = 0; i < missing_chains.size(); i++) {
            missing_chains.at(i).print(i);
        }
        rz_cons_printf("  excess_chains:\n");
        for (size_t i = 0; i < excess_chains.size(); i++) {
            excess_chains.at(i).print(i);
        }
    }
}