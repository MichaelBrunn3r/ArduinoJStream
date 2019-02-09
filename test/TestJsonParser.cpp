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
            {" , \"akey\" : 1}", ", \"akey\" : 1}"},
        };

        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            INFO("json: " << it->first);

            MockStringStream stream = MockStringStream(it->first);
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
            INFO("json: " << it->first);

            MockStringStream stream = MockStringStream(it->first);
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
        INFO("json: " << it->first);

        MockStringStream stream = MockStringStream(it->first);
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

            {",\"\\\"akey\\\": 123\": 123}", "\\\"akey\\\": 123", "123}"}
        };

        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            INFO("json: " << std::get<0>(*it));

            MockStringStream stream = MockStringStream(std::get<0>(*it));
            parser.parse(&stream);

            String result = parser.nextKey();
            INFO("result: " << result);
            REQUIRE(result.equals(std::get<1>(*it)));

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
            {", \"\\\\\"thekey\" :321}", "\\\"thekey", "321}"},
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

            // Escaped chars
            {",\"\\akey\":}", "thekey", "}"},
            {",\"\\\"akey\":}", "thekey", "}"},
            {",\"thekey\\\":\": 1}", "thekey\\", "}"},
            {",\"\\\"thekey\\\":\":}", "\\\"thekey", "}"}
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

SCENARIO("JsonParser::ascend" , "[ascend]") {
    JsonParser parser;

    GIVEN("Successfull ascends") {
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
            INFO("json: " << std::get<0>(*it));

            MockStringStream stream = MockStringStream(std::get<0>(*it));
            parser.parse(&stream);
            REQUIRE(parser.ascend(std::get<1>(*it)));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(std::get<2>(*it)));
        }
    }

    GIVEN("Unsuccessfull ascends") {
        std::vector<std::tuple<const char*, size_t, const char*>> parse = {
            // Empty Json
            {"", 1, ""},
            {"", 2, ""},
            {"", 5, ""},

            // Ignore strings
            {"\"a String ]\"", 1, ""}
        };

        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            INFO("json: " << std::get<0>(*it));

            MockStringStream stream = MockStringStream(std::get<0>(*it));
            parser.parse(&stream);
            REQUIRE_FALSE(parser.ascend(std::get<1>(*it)));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(std::get<2>(*it)));
        }
    }
}