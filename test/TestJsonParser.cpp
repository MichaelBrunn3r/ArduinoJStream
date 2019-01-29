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

SCENARIO("Method 'skipUntilKey'", "[skipUntilKey]") {
    JsonParser parser;
    GIVEN("Json with matching key") {
        /** Json | key | resulting Json **/
        std::vector<std::tuple<const char*, const char*, const char*>> parse = {
            {"\"thekey\":}", "thekey", "}"},
            {"{\"thekey\":", "thekey", ""},
            {"{\"thekey\":}", "thekey", "}"},
            {"{\"\\\"thekey\":}", "\\\"thekey", "}"},
            {"{\"numbers\": [1,2,3,4,5],\"thekey\": \"thevalue\"}", "thekey", " \"thevalue\"}"}
        };
        
        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            INFO("json: " << std::get<0>(*it));
            CHECK_THAT(parser.skipUntilKey(std::get<0>(*it), std::get<1>(*it)), Catch::Matchers::Equals(std::get<2>(*it)));

            MockStringStream* stream = new MockStringStream(std::get<0>(*it));
            parser.skipUntilKey(stream, std::get<1>(*it));
            CHECK_THAT(stream->readString().c_str(), Catch::Matchers::Equals(std::get<2>(*it)));
        }
    }

    GIVEN("Json without matching key") {
        /** Json | key | resulting Json **/
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
            INFO("json: " << std::get<0>(*it));
            CHECK_THAT(parser.skipUntilKey(std::get<0>(*it), std::get<1>(*it)), Catch::Matchers::Equals(std::get<2>(*it)));

            MockStringStream* stream = new MockStringStream(std::get<0>(*it));
            parser.skipUntilKey(stream, std::get<1>(*it));
            CHECK_THAT(stream->readString(), Catch::Matchers::Equals(std::get<2>(*it)));
        }
    }
}