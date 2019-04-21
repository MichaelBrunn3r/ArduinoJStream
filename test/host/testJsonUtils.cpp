#include "catch.hpp"

#include <vector>
#include <iostream>
#include <utility>
#include <cstring>
#include <sstream>
#include <cmath>

#include <Arduino.h>
#include <JsonParser.h>

#define protected public
#define private   public
#include <Internals/JsonUtils.h>
#undef protected
#undef private

using namespace JStream;

TEST_CASE("::stol") {
    JsonParser parser();

    SECTION("valid longs") {
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

            long result = Internals::stol(str, defaultVal);

            REQUIRE(result == expectedVal);
        }
    }

    SECTION("invalid longs") {
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

            long result = Internals::stol(str, defaultVal);

            REQUIRE(result == defaultVal);
        }
    }
}

TEST_CASE("::stod") {
    JsonParser parser();

    SECTION("valid longs") {
        std::vector<std::tuple<const char*, double, double>> parse = {
            // Ints
            {"1", -1, 1},
            {"0", -1, 0},
            {"-1", 2, -1},
            {"1-", -1, 1},
            {"-123", -1, -123},

            // Decimals
            {"1.0", -1, 1},
            {"1.1", -1, 1.1},
            {"1.1234567", -1, 1.1234567},

            // Scientific notation
            {"1e2", -1, 100},
            {"1e+2", -1, 100},
            {"1e12", -1, 1000000000000},
            {"1e+12", -1, 1000000000000},
            {"1e-2", -1, 0.01},

            // Decimal + scientific notation
            {"1.23456e1", -1, 12.3456},
            {"1.23456e+1", -1, 12.3456},
            {"1.23456e-1", -1, 0.123456},

            // Ignore leading zeros
            {"0000123", -1, 123},
            {"-0000123", -1, -123},
            {"1e02", -1, 100},
            {"1e012", -1, 1000000000000},
            {"1e+02", -1, 100},
            {"1e-02", -1, 0.01},

            {"18446744073709551615", -1, 18446744073709551615.0}, // ULONG_MAX
            {"18446744073709551616", -1, 18446744073709551616.0}, // 1844674407370955161*10+6 = 0
            {"98446744073709551615", -1, 98446744073709551615.0},

            // Skip leading whitespace
            {"\r\n\t -1\r\n\t ", 2, -1},
            {"\r\n\t -1.2", -1, -1.2},

            // Stop parsing at invalid chars
            {"123 3    a", -1, 123},
            {"a123", -1, -1},
            {"12a3", -1, 12},
            {"123, 456", -1, 123},
            {"1 .2", -1, 1},
            {"1. 2", -1, -1},
        };

        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            const char* str = std::get<0>(*it);
            double defaultVal = std::get<1>(*it);
            double expectedVal = std::get<2>(*it);

            CAPTURE(str);

            double result = Internals::stod(str, defaultVal);

            INFO("result: " << result);
            REQUIRE(std::fabs(expectedVal-result) <= 0.000000000001);
        }
    }
}