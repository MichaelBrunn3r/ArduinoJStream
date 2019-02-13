#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <vector>
#include <iostream>
#include <utility>
#include <cstring>

#include <Arduino.h>
#include <MockStringStream.h>

#define protected public
#define private   public
#include <JsonParser.h>
#undef protected
#undef private

using namespace JStream;

SCENARIO("JsonParser::atEnd", "[atEnd]") {
    JsonParser parser;

    GIVEN("Json not at the end of the current object/array") {
        std::vector<std::pair<const char*, const char*>> parse = {         
            // Skip whitespace in Arrays
            {",1]", ",1]"},
            {" ,1]", ",1]"},
            {", 1]", ", 1]"},
            {" , 1 ]", ", 1 ]"},
            {" 1]", "1]"},

            // Skip whitespace in Objects
            {",\"akey\": 1}", ",\"akey\": 1}"},
            {" ,\"akey\": 1}", ",\"akey\": 1}"},
            {", \"akey\": 1}", ", \"akey\": 1}"},
            {" , \"akey\" : 1}", ", \"akey\" : 1}"}
        };

        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            const char* json = it->first;

            INFO("json: " << json);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);
            REQUIRE(parser.atEnd());
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(it->second));
        }
    }
    
    GIVEN("Json at the end of the current object/array") {
        std::vector<std::pair<const char*, const char*>> parse = {
            {"", ""},
            {" ", ""},
            {"}", "}"},
            {"  }", "}"},
            {"]", "]"},
            {"  ]", "]"}
        };

        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            const char* json = it->first;

            INFO("json: " << json);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);
            REQUIRE_FALSE(parser.atEnd());
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(it->second));
        }
    }
}

SCENARIO("JsonParser::nextVal", "[nextVal]") {
    JsonParser parser;

    std::vector<std::pair<const char*, const char*>> parse = {
        {",1,2]", "1,2]"},
        {"1,2]", "2]"},
        {"1, 2]", "2]"},
        {"1, 2", "2"},

        {"[1,2,3], 2]", "2]"},
        {"{\"1\": 1}, 2]", "2]"}
    };

    for(auto it = parse.begin(); it!=parse.end(); ++it) {
        const char* json = it->first;

        INFO("json: " << json);

        MockStringStream stream = MockStringStream(json);
        parser.parse(&stream);
        REQUIRE(parser.nextVal());
        CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(it->second));
    }
}

SCENARIO("JsonParser::nextKey", "[nextKey]") {
    JsonParser parser;

    GIVEN("Next Key exists") {
        // Json string | result of 'nextKey' | resulting Json string
        std::vector<std::tuple<const char*, const char*, const char*>> parse = {
            {",\"akey\": 123}", "akey", "123}"},

            {",\"\\\"akey\\\": 123\": 123}", "\"akey\": 123", "123}"}
        };

        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            const char* json = std::get<0>(*it);
            const char* key = std::get<1>(*it);

            INFO("json: " << json);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);
            REQUIRE(parser.nextKey() == String(key));

            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(std::get<2>(*it)));
        }
    }

    GIVEN("No next Key exists") {
        // Json string | result of 'nextKey' | resulting Json string
        std::vector<std::tuple<const char*, const char*, const char*>> parse = {
            {"", "", ""},
            {"123", "", ""},

            // Stops at end of object/array
            {"123}123", "", "}123"}, // Stops at }
            {"123]123", "", "]123"}, // Stops at ]

            // Malformed key
            {",\"akey\" 123}", "", "123}"},
            {"\"akey\" 123}", "", "}"},

            // Inside last key of object
            {"\"akey\": 123}", "", "}"},

            {", 1, 2, 3]", "", "1, 2, 3]"},
            {", 1, 2, 3}", "", "1, 2, 3}"}
        };

        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            INFO("json: " << std::get<0>(*it));

            MockStringStream stream = MockStringStream(std::get<0>(*it));
            parser.parse(&stream);
            REQUIRE(parser.nextKey().equals(std::get<1>(*it)));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(std::get<2>(*it)));
        }
    }
}

SCENARIO("JsonParser::findKey", "[findKey]") {
    JsonParser parser;
    GIVEN("Json with matching key") {
        // Json | key | resulting Json
        std::vector<std::tuple<const char*, const char*, const char*>> parse = {
            // Test Whitespaces
            {",\"thekey\": 123}", "thekey", "123}"},
            {",\"thekey\"  : 123}", "thekey", "123}"},
            {",   \"thekey\"    :    123  }  ", "thekey", "123  }  "},

            // Skip matching key nested objects/arrays
            {", \"1\": {\"thekey\": 123}, \"thekey\": 321}", "thekey", "321}"},
            {", \"1\": [\"thekey\"], \"thekey\": 321}", "thekey", "321}"},
            {", \"numbers\": [1,2,3,4,5],\"thekey\": \"thevalue\"}", "thekey", "\"thevalue\"}"},

            // Skip empty key-value pairs
            {",, \"thekey\": 1", "thekey", "1"},
            {", , \"thekey\": 1", "thekey", "1"},
            {",\"1\": 1 ,, \"thekey\": 1", "thekey", "1"},

            // Skip malformed keys
            {",\"thekey\" 1, \"thekey\": 2}", "thekey", "2}"},
            {"\"thekey\" 1, \"thekey\": 2}", "thekey", "2}"},

            // Don't match prefix
            {",\"the\": 1, \"thekey\": 2}", "thekey", "2}"},
            {",\"thekey\": 1, \"thekeylong\": 2}", "thekeylong", "2}"},

            // Skip over short keys
            {",\"the\": 1, \"thekey\": 2}", "thekey", "2}"},

            // Match escaped chars
            {", \"\\\\thekey\" :321}", "\\thekey", "321}"},
            {", \"\\\\\\\"thekey\" :321}", "\\\"thekey", "321}"},
            {", \"\\\"thekey\" :321}", "\"thekey", "321}"},
            {", \"\\nthekey\" :321}", "\nthekey", "321}"}
        };
        
        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            const char* json = std::get<0>(*it);
            const char* thekey = std::get<1>(*it);

            INFO("json: " << json);        
            INFO("key: " << thekey);   

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);
            bool success = parser.findKey(thekey);
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(std::get<2>(*it)));
            REQUIRE(success);
        }
    }

    GIVEN("Json without matching key") {
        // Json | key | resulting Json
        std::vector<std::tuple<const char*, const char*, const char*>> parse = {
            {"", "", ""},
            {"", "thekey", ""},
            {"\"", "thekey", ""},
            {"\"\"", "thekey", ""},
            {"thekey", "thekey", ""},
            {",\"thekey\", suffix", "thekey", ""},
            {",\"akey\":\"notakey\"}", "notakey", "}"},
            

            // Skip matching key in nested object/array
            {", \"1\": {\"thekey\": 123}}", "thekey", "}"},
            {", \"1\": [\"thekey\"]}", "thekey", "}"},

            // Don't match prefixes
            {",\"thekey123\": 1}", "thekey", "}"},

            // Correctly escaped chars
            {",\"\\akey\":}", "thekey", "}"},
            {",\"\\\"akey\":}", "thekey", "}"},
            {",\"thekey\\\":\": 1}", "thekey\\", "}"},
            {",\"\\\"thekey\\\":\":}", "\\\"thekey", "}"},

            // Incorrectly and unescapeable chars
            {",\"\"thekey\": 1}", "\"thekey", ""},

            // EOF
            {", \"thekey\\", "thekey", ""},
            {", \"thekey", "thekey", ""},
            {", \"thekey\"}", "thekey", "}"}
        };
        
        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            const char* json = std::get<0>(*it);
            const char* thekey = std::get<1>(*it);

            INFO("json: " << json);
            INFO("key: " << thekey);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);
            bool success = parser.findKey(thekey);
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(std::get<2>(*it)));
            REQUIRE_FALSE(success);
        }
    }
}

SCENARIO("JsonParser::exit" , "[exit]") {
    JsonParser parser;

    GIVEN("Successfull exits") {
        std::vector<std::tuple<const char*, size_t, const char*>> parse = {
            {"123", 0, "123"},

            // One level
            {"]", 1, ""},
            {"}", 1, ""},

            // Two levels
            {"]]", 2, ""},
            {"]}", 2, ""},
            {"}}", 2, ""},
            {"}]", 2, ""},

            // Skip over remaining keys/values
            {"\"1\": 1, \"2\": 2, \"3\": 3}, suffix", 1, ", suffix"},
            {"1, 2, 3], suffix", 1, ", suffix"},

            // Ignore strings
            {"\"]}\", 123], 123", 1, ", 123"},
            {"\"]}\", 123}, 123", 1, ", 123"},

            // Skip nested objects/arrays
            {",[1,2,3],[7,8,9]}, suffix", 1, ", suffix"},
            {",{1,2,3},{7,8,9}}, suffix", 1, ", suffix"},
            {",[1,2,3],[7,8,9]], suffix", 1, ", suffix"},
            {",{1,2,3},{7,8,9}], suffix", 1, ", suffix"}
        };

        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            const char* json = std::get<0>(*it);
            int levels = std::get<1>(*it);

            INFO("json: " << json);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);
            REQUIRE(parser.exit(levels));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(std::get<2>(*it)));
        }
    }

    GIVEN("Unsuccessfull exits") {
        std::vector<std::tuple<const char*, size_t, const char*>> parse = {
            // Empty Json
            {"", 1, ""},
            {"", 2, ""},
            {"", 5, ""},

            // Ignore strings
            {"\"a String ]\"", 1, ""}
        };

        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            const char* json = std::get<0>(*it);
            int levels = std::get<1>(*it);

            INFO("json: " << json);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);
            REQUIRE_FALSE(parser.exit(levels));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(std::get<2>(*it)));
        }
    }
}

SCENARIO("JsonParser::enter", "[enter]") {
    JsonParser parser;

    const char* json = "\"1\": {}";

    MockStringStream stream = MockStringStream(json);
    parser.parse(&stream);
}

/////////////////////
// Private methods //
/////////////////////

SCENARIO("JsonParser::next", "[private, next]") {
    JsonParser parser;

    GIVEN("Json with next") {
        std::vector<std::tuple<const char*, size_t, const char*>> parse = {
            // immediate next
                {",1,2,3,4]", 1, "1,2,3,4]"},
                {", \"1\": 1, \"2\": 2, \"3\": 3, \"4\": 4}", 1, "\"1\": 1, \"2\": 2, \"3\": 3, \"4\": 4}"},

            // 2nd next
                {",1,2,3,4]", 2, "2,3,4]"},
                {", \"1\": 1, \"2\": 2, \"3\": 3, \"4\": 4}", 2, "\"2\": 2, \"3\": 3, \"4\": 4}"},

            // 3rd next
                {",1,2,3,4]", 3, "3,4]"},
                {", \"1\": 1, \"2\": 2, \"3\": 3, \"4\": 4}", 3, "\"3\": 3, \"4\": 4}"},

            // Skips over nested objects/arrays
                {"[1], 2]", 1, "2]"},
                {"[[1],[2]], 3]", 1, "3]"},
                {"\"1\": {\"1.1\": 1.1}, \"2\": 2}", 1, "\"2\": 2}"},
                {"{\"1.1\": 1.1}, \"2\": 2}", 1, "\"2\": 2}"},

                {",[1], 2]", 2, "2]"},
                {",[[1],[2]], 3]", 2, "3]"},

            // Strings
                {",\"\\\"akey\\\": 123\": 123}", 1, "\"\\\"akey\\\": 123\": 123}"}
        };

        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            const char* json = std::get<0>(*it);
            size_t n = std::get<1>(*it);

            INFO("json: " << json);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);
            REQUIRE(parser.next(n));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(std::get<2>(*it)));
        }
    }

    GIVEN("Json without next") {
        std::vector<std::tuple<const char*, size_t, const char*>> parse = {
            {"", 1, ""},
            {"123", 1, ""},
            {"}", 1, "}"},

            // Stops at end of object/array
                {"123}123", 1, "}123"}, // Stops at }
                {"123]123", 1, "]123"}, // Stops at ]
                {"\"akey\": 123}", 1, "}"},

            // Skips over nested objects/arrays
                {"{\"1\": {\"1.1\": 2}} }", 1, "}"},
                {"[1,[2,3]] ]", 1, "]"},

            // no n-th succeding element
                {"]", 1, "]"},
                {"}", 1, "}"},
                {",1,2]", 3, "]"},
                {", \"1\": 1, \"2\": 2}", 3, "}"}
        };

        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            const char* json = std::get<0>(*it);
            size_t n = std::get<1>(*it);

            INFO("json: " << json);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);
            REQUIRE_FALSE(parser.next(n));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(std::get<2>(*it)));
        }
    }
}

SCENARIO("JsonParser::skipWhitespace", "[private, skipWhitespace]") {
    JsonParser parser;

    std::vector<std::tuple<const char*, const char*>> parse = {
            {"", ""},
            {"\t\n\r ", ""},
            {"\"\t\n\r \"", "\"\t\n\r \""},
            {"  ,", ","}
        };

        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            const char* json = std::get<0>(*it);

            INFO("json: " << json);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);
            parser.skipWhitespace();
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(std::get<1>(*it)));
        }
}

SCENARIO("JsonParser::readString", "[private, readString]") {
    JsonParser parser;

    std::vector<std::tuple<const char*, const char*, const char*>> parse = {
        {"", "", ""},
        {"astring", "astring", ""},
        {"astring\", suffix", "astring", ", suffix"},

        // Escaped chars
        {"a\\\"string\", suffix", "a\"string", ", suffix"},
        {"a\\\\string\", suffix", "a\\string", ", suffix"},
        {"a\\nstring\", suffix", "a\nstring", ", suffix"}
    };

    for(auto it = parse.begin(); it!=parse.end(); ++it) {
        const char* json = std::get<0>(*it);
        String str = std::get<1>(*it);

        INFO("json: " << json);

        MockStringStream stream = MockStringStream(json);
        parser.parse(&stream);
        REQUIRE(parser.readString() == str);
        CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(std::get<2>(*it)));
    }
}