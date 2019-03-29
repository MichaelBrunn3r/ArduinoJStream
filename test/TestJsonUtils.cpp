#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <vector>
#include <iostream>
#include <utility>
#include <cstring>
#include <sstream>
#include <cmath>

#include <Arduino.h>
#include <JsonParser.h>
#include <MockStringStream.h>

#define protected public
#define private   public
#include <JsonUtils.h>
#undef protected
#undef private

SCENARIO("stol") {
    JStream::JsonParser parser;

    GIVEN("valid longs") {
        std::vector<std::tuple<const char*, long, long>> parse = {
            {"0", -1, 0},
            {"123", -1, 123},
            {"-123", -1, -123},

            // Stop parsing at invalid chars
            {"-123-45", -1, -123},
            {"123a45", -1, 123},

            // Skip leading whitespace
            {"\r\n\t -123", -1, -123},
        };

        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            const char* str = std::get<0>(*it);
            long defaultVal = std::get<1>(*it);
            long expectedVal = std::get<2>(*it);

            CAPTURE(str);

            long result = JStream::stol(str, defaultVal);

            REQUIRE(result == expectedVal);
        }
    }

    GIVEN("invalid longs") {
        std::vector<std::tuple<const char*, long>> parse = {
            {"", -1},

            // Stop parsing at invalid chars
            {"--123-45", -1},
            {"a123", -1},

            // Skip leading whitespace
            {"\r\n\t ", -1},
        };

        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            const char* str = std::get<0>(*it);
            long defaultVal = std::get<1>(*it);

            CAPTURE(str);

            long result = JStream::stol(str, defaultVal);

            REQUIRE(result == defaultVal);
        }
    }
}