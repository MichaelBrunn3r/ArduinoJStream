#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <vector>
#include <iostream>
#include <utility>
#include <cstring>

#include <Arduino.h>
#include <JsonParser.h>
#include <MockStringStream.h>

using namespace JStream;

SCENARIO("JsonParser::skipString") {
    GIVEN("Outside String") {
        std::vector<std::pair<const char*, const char*>> parse = {
            {"\"astring\"", ""},
            {"\"astring\" suffix", " suffix"},
            {"prefix \"astring\" suffix", " suffix"},
            {"prefix \"\\\"astring\" suffix", " suffix"}
        };

        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            INFO("json: " << it->first);

            MockStringStream* stream = new MockStringStream(it->first);
            JsonParser::skipString(stream, false);
            CHECK_THAT(stream->readString().c_str(), Catch::Matchers::Equals(it->second));
        }
    }

    GIVEN("Inside String") {
        std::vector<std::pair<const char*, const char*>> parse = {
            {"astring\"", ""},
            {"astring\" suffix", " suffix"},
            {"\\\"astring\" suffix", " suffix"},
            {"a\\\"string\" suffix", " suffix"}
        };

        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            INFO("json: " << it->first);

            MockStringStream* stream = new MockStringStream(it->first);
            JsonParser::skipString(stream, true);
            CHECK_THAT(stream->readString().c_str(), Catch::Matchers::Equals(it->second));
        }
    }
}

SCENARIO("JsonParser::skipUntilKey", "[skipUntilKey]") {
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
            CHECK_THAT(JsonParser::skipUntilKey(std::get<0>(*it), std::get<1>(*it)), Catch::Matchers::Equals(std::get<2>(*it)));

            MockStringStream* stream = new MockStringStream(std::get<0>(*it));
            JsonParser::skipUntilKey(stream, std::get<1>(*it));
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
            CHECK_THAT(JsonParser::skipUntilKey(std::get<0>(*it), std::get<1>(*it)), Catch::Matchers::Equals(std::get<2>(*it)));

            MockStringStream* stream = new MockStringStream(std::get<0>(*it));
            JsonParser::skipUntilKey(stream, std::get<1>(*it));
            CHECK_THAT(stream->readString(), Catch::Matchers::Equals(std::get<2>(*it)));
        }
    }
}

SCENARIO("JsonParser::nextEntry", "[nextEntry]") {
    std::vector<std::pair<const char*, const char*>> parse = {
        {",\"2\": 2", "\"2\": 2"},
        {"\"avalue\" ,\"2\": 2", "\"2\": 2"},
        {"[1,2,3] ,\"2\": 2", "\"2\": 2"},
        {"{\"1\": 1, \"2\": 2} ,\"2\": 2", "\"2\": 2"},
        {"}]}]}", "}]}]}"},
        {"]]]}]", "]]]}]"}
    };

    for(auto it = parse.begin(); it!=parse.end(); ++it) {
        INFO("json: " << it->first);

        MockStringStream* stream = new MockStringStream(it->first);
        JsonParser::nextEntry(stream);
        CHECK_THAT(stream->readString().c_str(), Catch::Matchers::Equals(it->second));
    }
}