#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <vector>
#include <iostream>
#include <utility>
#include <cstring>

#include <Arduino.h>
#include <JsonParser.h>
#include <Streams/StringStream.h>
#include <CharBuffer.h>
#include <MockStringStream.h>

using namespace JStream;

SCENARIO("Method 'skipUntilKey(const char*, const char*)'", "[skipUntilKey]") {
    JsonParser parser;
    GIVEN("Json with matching key") {
        std::vector<std::tuple<const char*, const char*, const char*>> parse = {
            {"\"thekey\":}", "thekey", "}"},
            {"{\"thekey\":", "thekey", ""},
            {"{\"thekey\":}", "thekey", "}"},
            {"{\"\\\"thekey\":}", "\\\"thekey", "}"}
        };
        
        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            const char* result = parser.skipUntilKey(std::get<0>(*it), std::get<1>(*it));
            INFO("json: " << std::get<0>(*it));
            CHECK_THAT(result, Catch::Matchers::Equals(std::get<2>(*it)));
        }
    }

    GIVEN("Json without matching key") {
        std::vector<std::tuple<const char*, const char*, const char*>> parse = {
            {"", "thekey", ""},
            {"", "", ""},
            {"\"", "thekey", ""},
            {"\"\"", "thekey", ""},
            {"thekey", "thekey", ""},
            {"{\"akey\":\"notakey\"}", "notakey", ""},
            {"{\"\\\"notakey\":}", "notakey", ""},
            {"{\"\\\"notakey\\\":\":}", "\\\"notakey", ""}
        };
        
        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            const char* result = parser.skipUntilKey(std::get<0>(*it), std::get<1>(*it));
            INFO("json: " << std::get<0>(*it));
            CHECK_THAT(result, Catch::Matchers::Equals(std::get<2>(*it)));
        }
    }
}