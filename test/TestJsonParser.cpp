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
            const char* json_after_exec = it->second;

            INFO("json: " << json);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);
            REQUIRE(parser.atEnd());
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
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
            const char* json_after_exec = it->second;

            INFO("json: " << json);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);
            REQUIRE_FALSE(parser.atEnd());
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
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
        const char* json_after_exec = it->second;

        INFO("json: " << json);

        MockStringStream stream = MockStringStream(json);
        parser.parse(&stream);
        REQUIRE(parser.nextVal());
        CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
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
            const char* expected_key = std::get<1>(*it);
            const char* json_after_exec = std::get<2>(*it);

            INFO("json: " << json);

            // Test capturing the key
            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);
            String key = "";
            REQUIRE(parser.nextKey(&key));
            CHECK_THAT(key.c_str(), Catch::Matchers::Equals(expected_key));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));

            // Test not capturing the key
            stream = MockStringStream(json);
            parser.parse(&stream);
            REQUIRE(parser.nextKey(NULL));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }

    GIVEN("No next Key exists") {
        // Json string | result of 'nextKey' | resulting Json string
        std::vector<std::tuple<const char*, const char*>> parse = {
            {"", ""},
            {"123", ""},

            // Stops at end of object/array
            {"123}123", "}123"}, // Stops at }
            {"123]123", "]123"}, // Stops at ]

            // Skips malformed keys
            {",\"akey\" 123}", "123}"},
            {"\"akey\" 123}", "}"},

            // Inside last key of object
            {"\"akey\": 123}", "}"},

            {", 1, 2, 3]", "1, 2, 3]"},
            {", 1, 2, 3}", "1, 2, 3}"}
        };

        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            const char* json = std::get<0>(*it);
            const char* json_after_exec = std::get<1>(*it);

            INFO("json: " << json);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);

            REQUIRE_FALSE(parser.nextKey());
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
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
            {",\n\t\r \"thekey\"\n\t\r :\n\t\r 123\n\t\r }  ", "thekey", "123\n\t\r }  "},

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

            // Skip keys that are prefix of thekey
            {",\"the\": 1, \"thekey\": 2}", "thekey", "2}"},

            // Match escaped chars
            {", \"\\\\thekey\" :321}", "\\thekey", "321}"},
            {", \"\\\\\\\"thekey\" :321}", "\\\"thekey", "321}"},
            {", \"\\\"thekey\" :321}", "\"thekey", "321}"},
            {", \"\\nthekey\" :321}", "\nthekey", "321}"},

            // Match current key. Don't advance to the next key
            {"\"thekey\": 123, \"akey\": 312}", "thekey", "123, \"akey\": 312}"},
            {"\n\t\r \"thekey\": 123, \"akey\": 312}", "thekey", "123, \"akey\": 312}"}
        };
        
        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            const char* json = std::get<0>(*it);
            const char* thekey = std::get<1>(*it);
            const char* json_after_exec = std::get<2>(*it);

            INFO("json: " << json);        
            INFO("key: " << thekey);   

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);
            bool success = parser.findKey(thekey);
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
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

            // End of Stream
            {", \"thekey\\", "thekey", ""},
            {", \"thekey", "thekey", ""},
            {", \"thekey\"}", "thekey", "}"}
        };
        
        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            const char* json = std::get<0>(*it);
            const char* thekey = std::get<1>(*it);
            const char* json_after_exec = std::get<2>(*it);

            INFO("json: " << json);
            INFO("key: " << thekey);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);
            bool success = parser.findKey(thekey);
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
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
            size_t levels = std::get<1>(*it);
            const char* json_after_exec = std::get<2>(*it);

            INFO("json: " << json);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);
            REQUIRE(parser.exit(levels));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
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
            size_t levels = std::get<1>(*it);
            const char* json_after_exec = std::get<2>(*it);

            INFO("json: " << json);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);
            REQUIRE_FALSE(parser.exit(levels));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }
}

SCENARIO("JsonParser::find", "[find]") {
    JsonParser parser;

    GIVEN("Existing path") {
        std::vector<std::tuple<const char*, const char*, const char*>> parse = {
            {"", "", ""},
            {"\"akey\": 1", "", "\"akey\": 1"},

            // Find key
            {",\"thekey\": 1}", "thekey", "1}"},
            {"\"thekey\": 1}", "thekey", "1}"},
            {"\"akey\": 1, \"thekey\": 2}", "thekey", "2}"},
            {", \"akey\": 1, \"thekey\": 2}", "thekey", "2}"},
            
            // Find array element
            {", 1, 2, 3, 4]", "[0]", ", 1, 2, 3, 4]"},
            {"0, 1, 2, 3, 4]", "[0]", "0, 1, 2, 3, 4]"},
            {"0, 1, 2, 3, 4]", "[1]", "1, 2, 3, 4]"},
            {"0, 1, 2, 3, 4]", "[2]", "2, 3, 4]"},
            {"0, 1, 2, 3, 4]", "[3]", "3, 4]"},
            {"0, 1, 2, 3, 4]", "[4]", "4]"},

            // Find nested element
            {"\"obj1\": {\"thekey\": 1}}", "obj1/thekey", "1}}"},
            {"\"obj1\": {\"obj2\": {\"thekey\": 1}}}", "obj1/obj2/thekey", "1}}}"},
            {"[1,2,3], [4,5,6], [7,8,9]]", "[2][2]", "9]]"},
            {", [1,2,3], [4,5,6], [7,8,9]]", "[2][2]", "6], [7,8,9]]"},
            {"\"arr1\": [1, 2, 3]", "arr1[1]", "2, 3]"},
            {"\"arr1\": [1,2,3], \"arr2\": [1, 2, 3]", "arr2[1]", "2, 3]"},
            {"\"arr1\": [[1,2,3], [4,5,6], [7,8,9]]", "arr1[1][1]", "5,6], [7,8,9]]"},
            {"\"obj1\": {\"arr1\": [1, 2, 3]}", "obj1/arr1[1]", "2, 3]}"},
            {"\"obj1\": {\"arr1\": [{\"akey\": 1}, {\"akey\": 1, \"thekey\": 2}, {\"akey\": 1}]}", "obj1/arr1[1]/thekey", "2}, {\"akey\": 1}]}"},

            // Escaped chars
            {",\"\\\"thekey\\\"\": 1}", "\"thekey\"", "1}"},
        };

        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            const char* json = std::get<0>(*it);
            const char* path_str = std::get<1>(*it);
            const char* json_after_exec = std::get<2>(*it);

            INFO("json: " << json);
            INFO("path: " << path_str);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);
            std::vector<JsonParser::PathSegment> path;
            REQUIRE(parser.compilePath(path, path_str)); 
            REQUIRE(parser.find(path));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }

    GIVEN("Non existing path") {
        std::vector<std::tuple<const char*, const char*, const char*>> parse = {
            // Index out of bounds
            {"0, 1, 2, 3, 4]", "[5]", "]"},
            {"[1,2,3], [3,4,5], [6,7,8]]", "[1][3]", "], [6,7,8]]"},
        };

        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            const char* json = std::get<0>(*it);
            const char* path_str = std::get<1>(*it);
            const char* json_after_exec = std::get<2>(*it);

            INFO("json: " << json);
            INFO("path: " << path_str);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);
            std::vector<JsonParser::PathSegment> path;
            REQUIRE(parser.compilePath(path, path_str)); 
            REQUIRE_FALSE(parser.find(path));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }
    
}

SCENARIO("JsonParser::compilePath", "[compilePath]") {
    JsonParser parser;

    GIVEN("Valid paths") {
        std::vector<std::tuple<const char*, std::vector<JsonParser::PathSegment>>> parse = {
            {"thekey", {{"thekey"}}},
            {"obj1/thekey", {{"obj1"}, {"thekey"}}},
            {"[42]", {{42}}},

            // Escaped chars
            {"\\\"thekey", {{"\\\"thekey"}}},
            {"\\/thekey\\/", {{"/thekey/"}}},
            {"\\[\\]thekey\\[\\]", {{"[]thekey[]"}}},
            {"\\]\\[thekey\\]\\[", {{"][thekey]["}}},
            
            // Whitespace
            {"\n\r\t thekey \n\r\t ", {{"\n\r\t thekey \n\r\t "}}},
            {"\n\r\t obj1 \n\r\t /\n\r\t thekey \n\r\t ", {{"\n\r\t obj1 \n\r\t "}, {"\n\r\t thekey \n\r\t "}}},

            // Nesting
            {"obj1/obj2/obj3", {{"obj1"}, {"obj2"}, {"obj3"}}},
            {"[1][2][3]", {{1}, {2}, {3}}},
            {"obj[1][2][3]", {{"obj"}, {1}, {2}, {3}}},
            {"obj1[1]/obj2[2]/obj3[3]/obj4", {{"obj1"}, {1}, {"obj2"}, {2}, {"obj3"}, {3}, {"obj4"}}},
        };

        for(auto it=parse.begin(); it!=parse.end(); ++it) {
            const char* path = std::get<0>(*it);
            std::vector<JsonParser::PathSegment> expected_vec = std::get<1>(*it);

            INFO("path: " << path);

            std::vector<JsonParser::PathSegment> vec;
            REQUIRE(parser.compilePath(vec, path));
            REQUIRE(vec.size() == expected_vec.size());
            for(size_t i=0; i<vec.size(); i++) {
                REQUIRE(vec.at(i).type == expected_vec.at(i).type);
                if(vec.at(i).type == JsonParser::PathSegmentType::OFFSET) {
                    REQUIRE(vec.at(i).val.offset == expected_vec.at(i).val.offset);
                } else {
                    CHECK_THAT(vec.at(i).val.key, Catch::Matchers::Equals(expected_vec.at(i).val.key));
                }
            }
        }
    }

    GIVEN("Invalid paths") {
        std::vector<std::tuple<const char*>> parse = {
            {"obj1[1]obj2"}
        };

        for(auto it=parse.begin(); it!=parse.end(); ++it) {
            const char* path = std::get<0>(*it);

            INFO("path: " << path);

            std::vector<JsonParser::PathSegment> vec;
            REQUIRE_FALSE(parser.compilePath(vec, path));
        }
    }

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
            const char* json_after_exec = std::get<2>(*it);

            INFO("json: " << json);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);
            REQUIRE(parser.next(n));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
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
            const char* json_after_exec = std::get<2>(*it);

            INFO("json: " << json);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);
            REQUIRE_FALSE(parser.next(n));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
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
            const char* json_after_exec = std::get<1>(*it);

            INFO("json: " << json);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);
            parser.skipWhitespace();
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
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
        const char* expected_str = std::get<1>(*it);
        const char* json_after_exec = std::get<2>(*it);

        INFO("json: " << json);

        MockStringStream stream = MockStringStream(json);
        parser.parse(&stream);
        String str = "";
        parser.readString(str);
        CHECK_THAT(str.c_str(), Catch::Matchers::Equals(expected_str));
        CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
    }
}