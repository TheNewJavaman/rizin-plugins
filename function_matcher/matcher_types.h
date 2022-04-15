#pragma once

#include <string>
#include <vector>
#include <map>

namespace function_matcher {
    typedef std::vector<int8_t> buffer_t;

    struct instruction_t {
    public:
        buffer_t op;
        buffer_t payload;

        bool operator<(const instruction_t& other) const;
    };

    struct matcher_t {
    public:
        std::map<instruction_t, uint32_t> instruction_counts;
        std::vector<instruction_t> instructions;

        void print(const std::string& name) const;
    };
}