#pragma once

#include <string>
#include <vector>
#include <map>

namespace function_matcher {
    typedef std::vector<int8_t> buffer_t;

    struct instruction_t {
    public:
        size_t op_len;
        buffer_t buffer;
        buffer_t mask;

        bool operator<(const instruction_t& other) const;
    };

    struct matcher_t {
    public:
        size_t bytes;
        std::map<instruction_t, uint32_t> instruction_counts;
        std::vector<instruction_t> instructions;

        void print(const std::string& name) const;
    };
}