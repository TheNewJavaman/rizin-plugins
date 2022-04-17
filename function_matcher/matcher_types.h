#pragma once

#include <string>
#include <vector>
#include <map>

namespace fnm {
    typedef std::vector<uint8_t> buffer_t;

    struct instruction_t {
        size_t addr;
        int32_t bytes;
        buffer_t buffer;
        buffer_t mask;

    public:
        bool operator<(const instruction_t &other) const;

        bool operator==(const instruction_t &other) const;
    };

    typedef std::map<instruction_t, uint32_t> instruction_count_t;

    struct matcher_t {
        int32_t bytes;
        instruction_count_t instruction_counts;
        std::vector<instruction_t> instructions;

    public:
        void print(const std::string &name) const;
    };

    struct chain_common_t {
        size_t addr;
        int32_t bytes;
        size_t start_orig;
        size_t start_this;
        int32_t len;

    public:
        bool operator==(const chain_common_t &other) const;

        void print(size_t i) const;
    };

    struct chain_missing_t {
        size_t start_orig;
        int32_t len;

    public:
        void print(size_t i) const;
    };

    struct chain_excess_t {
        size_t addr;
        int32_t bytes;
        size_t start_this;
        int32_t len;

    public:
        void print(size_t i) const;
    };

    struct match_t {
        size_t addr;
        int32_t bytes;
        int32_t len;
        float byte_coverage;
        float instruction_coverage;
        std::vector<chain_common_t> common_chains;
        std::vector<chain_missing_t> missing_chains;
        std::vector<chain_excess_t> excess_chains;

    public:
        bool has_common_chain(const chain_common_t &chain) const;

        void calc_coverage(const matcher_t &matcher);

        void print() const;
    };
}