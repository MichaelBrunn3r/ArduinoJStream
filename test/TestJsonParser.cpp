#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <vector>
#include <iostream>
#include <utility>
#include <cstring>
#include <sstream>
#include <cmath>

#include <Arduino.h>
#include <MockStringStream.h>

#define protected public
#define private   public
#include <JsonParser.h>
#undef protected
#undef private

#include <Path.h>

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

            CAPTURE(json);

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

            CAPTURE(json);

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

        CAPTURE(json);

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
            {"\"akey\": 123}", "akey", "123}"},
            {"\n\r\t \"akey\": 123}", "akey", "123}"},
            {",\"akey\": 123}", "akey", "123}"},

            // Read escpaped chars
            {",\"\\\"akey\\\": 123\": 123}", "\"akey\": 123", "123}"},

            // Skip invalid key
            {", \"invalidkey\" 123, \"akey\": 123}", "akey", "123}"},
            {"\"invalidkey\" 123, \"akey\": 123}", "akey", "123}"},
            {", 1, 2, 3, \"akey\": 123}", "akey", "123}"},
            {", [1,2,3], {\"key1\": 123}, \"akey\": 123}", "akey", "123}"}
        };

        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            const char* json = std::get<0>(*it);
            const char* expected_key = std::get<1>(*it);
            const char* json_after_exec = std::get<2>(*it);

            CAPTURE(json);

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
            {"123}123", "123"}, // Stops at }
            {"123]123", "123"}, // Stops at ]
            {"\n\r\t }, 123", ", 123"}, // Stops at ]
            {"\n\r\t ], 123", ", 123"}, // Stops at ]

            // Skips malformed keys
            {",\"akey\" 123", ""},
            {"\"akey\" 123", ""},

            // Skip until end in arrays
            {", 1, 2, 3", ""},
            {", 1, 2, 3", ""}
        };

        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            const char* json = std::get<0>(*it);
            const char* json_after_exec = std::get<1>(*it);

            CAPTURE(json);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);

            REQUIRE_FALSE(parser.nextKey(NULL));
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

            CAPTURE(json, thekey);  

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
            {"}, suffix", "thekey", ", suffix"},
            {"], suffix", "thekey", ", suffix"},
            {"\"", "thekey", ""},
            {"\"\"", "thekey", ""},
            {"thekey", "thekey", ""},
            {",\"thekey\", suffix", "thekey", ""},
            {",\"akey\":\"notakey\"}", "notakey", ""},
            
            // Skip matching key in nested object/array
            {", \"1\": {\"thekey\": 123}}", "thekey", ""},
            {", \"1\": [\"thekey\"]}", "thekey", ""},

            // Don't match prefixes
            {",\"thekey123\": 1}", "thekey", ""},

            // Correctly escaped chars
            {",\"\\akey\":}", "thekey", ""},
            {",\"\\\"akey\":}", "thekey", ""},
            {",\"thekey\\\":\": 1}", "thekey\\", ""},
            {",\"\\\"thekey\\\":\":}", "\\\"thekey", ""},

            // Incorrectly and unescapeable chars
            {",\"\"thekey\": 1}", "\"thekey", ""},

            // End of Stream
            {", \"thekey\\", "thekey", ""},
            {", \"thekey", "thekey", ""},
            {", \"thekey\"}", "thekey", ""}
        };
        
        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            const char* json = std::get<0>(*it);
            const char* thekey = std::get<1>(*it);
            const char* json_after_exec = std::get<2>(*it);

            CAPTURE(json, thekey);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);
            bool success = parser.findKey(thekey);
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
            REQUIRE_FALSE(success);
        }
    }
}

SCENARIO("JsonParser::exitCollection" , "[exitCollection]") {
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

            CAPTURE(json);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);
            REQUIRE(parser.exitCollection(levels));
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

            CAPTURE(json);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);
            REQUIRE_FALSE(parser.exitCollection(levels));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }
}

SCENARIO("JsonParser::skipCollection", "[skipCollection]") {
    JsonParser parser;

    GIVEN("Successfull skips") {
        std::vector<std::tuple<const char*, const char*>> parse = {
            {"[], suffix", ", suffix"},
            {"{}, suffix", ", suffix"},

            // Whitespaces
            {"\r\n\t [\r\n\t ], suffix", ", suffix"},
            {"\r\n\t {\r\n\t }, suffix", ", suffix"},

            // Skip nested objects/arrays
            {"{[1,2,3],[7,8,9]}, suffix", ", suffix"},
            {"{{1,2,3},{7,8,9}}, suffix", ", suffix"},
            {"[[1,2,3],[7,8,9]], suffix", ", suffix"},
            {"[{1,2,3},{7,8,9}], suffix", ", suffix"}
        };

        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            const char* json = std::get<0>(*it);
            const char* json_after_exec = std::get<1>(*it);

            CAPTURE(json);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);
            REQUIRE(parser.skipCollection());
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }
}

SCENARIO("JsonParser::readString & JsonParser::skipString", "[readString, skipString]") {
    JsonParser parser;

    GIVEN("valid strings") {
        std::vector<std::tuple<const char*, bool, const char*, const char*>> parse = {
            {"\"", true, "", ""},
            {"astring\", suffix", true, "astring", ", suffix"},
            {"\"astring\", suffix", false, "astring", ", suffix"},

            // Escaped chars
            {"a\\\"string\", suffix", true, "a\"string", ", suffix"},
            {"a\\\\string\", suffix", true, "a\\string", ", suffix"},
            {"a\\nstring\", suffix", true, "a\nstring", ", suffix"}
        };

        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            const char* json = std::get<0>(*it);
            const bool inStr = std::get<1>(*it);
            const char* expected_str = std::get<2>(*it);
            const char* json_after_exec = std::get<3>(*it);

            CAPTURE(json);

            // readString
            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);
            String str = "";
            REQUIRE(parser.readString(str, inStr));
            CHECK_THAT(str.c_str(), Catch::Matchers::Equals(expected_str));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));

            // skipString
            stream = MockStringStream(json);
            parser.parse(&stream);
            REQUIRE(parser.skipString(inStr));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }

    GIVEN("invalid strings") {
        std::vector<std::tuple<const char*, bool, const char*, const char*>> parse = {
            {"", true, "", ""},
            {"\"", false, "", ""},
            {"astring", true, "astring", ""},
            {"\"astring", false, "astring", ""},
        };

        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            const char* json = std::get<0>(*it);
            const bool inStr = std::get<1>(*it);
            const char* expected_str = std::get<2>(*it);
            const char* json_after_exec = std::get<3>(*it);

            CAPTURE(json);

            // readString
            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);
            String str = "";
            REQUIRE_FALSE(parser.readString(str, inStr));
            CHECK_THAT(str.c_str(), Catch::Matchers::Equals(expected_str));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));

            // skipString
            stream = MockStringStream(json);
            parser.parse(&stream);
            REQUIRE_FALSE(parser.skipString(inStr));
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

            CAPTURE(json, path_str);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);
            Path path = Path(path_str);
            REQUIRE(parser.find(path));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));

            stream = MockStringStream(json);
            parser.parse(&stream);
            REQUIRE(parser.find(path_str));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }

    GIVEN("Non existing path") {
        std::vector<std::tuple<const char*, const char*, const char*>> parse = {
            // Index out of bounds
            {"0, 1, 2, 3, 4]", "[5]", ""},
            {"[1,2,3], [3,4,5], [6,7,8]]", "[1][3]", ", [6,7,8]]"},
        };

        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            const char* json = std::get<0>(*it);
            const char* path_str = std::get<1>(*it);
            const char* json_after_exec = std::get<2>(*it);

            CAPTURE(json, path_str);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);

            Path path = Path(path_str);
            REQUIRE_FALSE(parser.find(path));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }
    
}

SCENARIO("JsonParser::parseInt") {
    JsonParser parser;

    GIVEN("Valid Ints") {
        std::vector<std::tuple<const char*, long, const char*>> parse {
            {"1", 1, ""},
            {"0", 0, ""},
            {"-1", -1, ""},
            {"1-", 1, "-"},
            {"123 3    a", 1233, "a"},

            // Whitespaces
            {"\r\n\t -\r\n\t 1\r\n\t ", -1, ""},
            {"\r\n\t -\r\n\t 1\r\n\t 2\r\n\t 3", -123, ""},
            {"\r\n\t -\r\n\t 1\r\n\t 2\r\n\t 3", -123, ""},

            // Ignore leading zeros
            {"0000123", 123, ""},
            {"000 000 000 000 123", 123, ""},
            {"-0000123", -123, ""},
        };

        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            const char* json = std::get<0>(*it);
            long expected_int = std::get<1>(*it);
            const char* json_after_exec = std::get<2>(*it);

            CAPTURE(json, json);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);

            long num = parser.parseInt();
            REQUIRE(num == expected_int);
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }

    GIVEN("Invalid Ints") {
        std::vector<std::tuple<const char*, long, const char*>> parse {
            {"", 0, ""}, // No digits
        };

        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            const char* json = std::get<0>(*it);
            long expected_int = std::get<1>(*it);
            const char* json_after_exec = std::get<2>(*it);

            CAPTURE(json);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);

            long num = parser.parseInt();
            REQUIRE(num == expected_int);
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }
}

SCENARIO("JsonParser::parseNum") {
    JsonParser parser;

    GIVEN("Valid Numbers") {
        std::vector<std::tuple<const char*, double, const char*>> parse {
            // Ints
            {"1", 1, ""},
            {"0", 0, ""},
            {"-1", -1, ""},
            {"1-", 1, "-"},
            {"-123", -123, ""},

            // Decimals
            {"1.0", 1, ""},
            {"1.1", 1.1, ""},
            {"1.1234567", 1.1234567, ""},
            {"\r\n\t -1.2", -1.2, ""},

            // Scientific notation
            {"1e2", 100, ""},
            {"1e+2", 100, ""},
            {"1e12", 1000000000000, ""},
            {"1e+12", 1000000000000, ""},
            {"1e-2", 0.01, ""},

            // Decimal + scientific notation
            {"1.23456e1", 12.3456, ""},
            {"1.23456e+1", 12.3456, ""},
            {"1.23456e-1", 0.123456, ""},

            // Ignore leading zeros
            {"0000123", 123, ""},
            {"-0000123", -123, ""},
            {"1e02", 100, ""},
            {"1e012", 1000000000000, ""},
            {"1e+02", 100, ""},
            {"1e-02", 0.01, ""},

            {"18446744073709551615", 18446744073709551615.0, ""}, // ULONG_MAX
            {"18446744073709551616", 18446744073709551616.0, ""}, // 1844674407370955161*10+6 = 0
            {"98446744073709551615", 98446744073709551615.0, ""},
        };

        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            const char* json = std::get<0>(*it);
            double expected_decimal = std::get<1>(*it);
            const char* json_after_exec = std::get<2>(*it);
            
            CAPTURE(json);
            INFO("expected: " << expected_decimal);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);

            double decimal = parser.parseNum();

            std::ostringstream sstream;
            sstream << decimal;
            std::string decimal_str = sstream.str();

            INFO("result: " << decimal_str);
            REQUIRE(std::fabs(expected_decimal-decimal) <= 0.000000000001);
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }

    GIVEN("Invalid numbers") {
        std::vector<std::tuple<const char*, double, const char*>> parse {
            // Ints
            {"123 3", 123, " 3"},
            {"123a3", 123, "a3"},

            // Decimals
            {"\r\n\t 1 .2", 1.0, " .2"},
            {"\r\n\t 1. 2", 0.0, " 2"},
        };

        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            const char* json = std::get<0>(*it);
            double expected_decimal = std::get<1>(*it);
            const char* json_after_exec = std::get<2>(*it);
            
            CAPTURE(json);
            INFO("expected: " << expected_decimal);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);

            double decimal = parser.parseNum();

            INFO("result: " << decimal);

            REQUIRE(std::fabs(expected_decimal-decimal) <= 0.000000000001);
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }
}

SCENARIO("Parse Int Array") {
    JsonParser parser;

    GIVEN("valid arrays") {
        std::vector<std::tuple<const char*, bool, std::vector<long>>> parse {
            {"[1,-2,3]", false, {1,-2,3}},
            {"[-1,2,-3]", false, {-1,2,-3}},
            {"[1,-2,3]", true, {1,-2,3}},
            {"1,-2,3]", true, {1,-2,3}},
            {"-1,2,-3]", true, {-1,2,-3}},
            {"[9268176913,-1409571945,128568915]", false, {9268176913,-1409571945,128568915}},

            // Whitespaces
            {"\n\r\t [\n\r\t 1\n\r\t ]\n\r\t ", false, {1}},
            {"[\n\r\t 1  \n\r\t ,\n\r\t -2\n\r\t ,\n\r\t 3\n\r\t ]", false, {1,-2,3}},
            {"[-\r\n\t 1, 2\r\n\t 3, 4abcdefghijklmnopqrstuvwxyz5]", false, {-1,23,45}},

            // Parse unordered int array: an array where each integer is a string 
            {"[\"2\", \"1\", \"3\"]", false, {2,1,3}},
            {"[\"2\", \"-1\", \"3\"]", false, {2,-1,3}},
            {"[\"-2\", \"-1\", \"-3\"]", false, {-2,-1,-3}},

            // Minus has to be in front of number
            {"[1-,2,3]", false, {1,2,3}},
            {"[1\r\n\t -,2,3]", false, {1,2,3}},

            // Don't save empty numbers
            {"[1,,,2]", false, {1,2}},
            {",,,1,2]", true, {1,2}},
        };

        for(auto it=parse.begin(); it!=parse.end(); ++it) {
            const char* json = std::get<0>(*it);
            bool inArray = std::get<1>(*it);
            std::vector<long> expected_vec = std::get<2>(*it);

            CAPTURE(json);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);
            
            std::vector<long> vec;
            REQUIRE(parser.parseIntArray<long>(vec, inArray));
            REQUIRE(vec.size() == expected_vec.size());

            for(int i=0; i<vec.size(); i++) {
                REQUIRE(vec.at(i) == expected_vec.at(i));
            }
        }
    }

    GIVEN("invalid arrays") {
        std::vector<std::tuple<const char*, bool, std::vector<long>>> parse {
            {"1,-2,3]", false, {}},
            {"[1,-2,3", false, {1, -2}},
            {"[1", false, {}},
            {"[-", false, {}},
        };

        for(auto it=parse.begin(); it!=parse.end(); ++it) {
            const char* json = std::get<0>(*it);
            bool inArray = std::get<1>(*it);
            std::vector<long> expected_vec = std::get<2>(*it);

            CAPTURE(json);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);
            
            std::vector<long> vec;
            REQUIRE_FALSE(parser.parseIntArray<long>(vec, inArray));
            REQUIRE(vec.size() == expected_vec.size());

            for(int i=0; i<vec.size(); i++) {
                REQUIRE(vec.at(i) == expected_vec.at(i));
            }
        }
    }
}

SCENARIO("Parse Num Array") {
    JsonParser parser;

    GIVEN("valid arrays") {
        std::vector<std::tuple<const char*, std::vector<double>>> parse {
            // Int arrays
            {"[1,-2,3]", {1,-2,3}},
            {"[-1,2,-3]", {-1,2,-3}},
            {"[9268176913,-1409571945,128568915]", {9268176913,-1409571945,128568915}},

            // Decimal arrays
            {"[-1.0,2.12314,4.35]", {-1.0,2.12314,4.35}},
            {"-1.0,2.12314,4.35]", {-1.0,2.12314,4.35}},

            // Scientific notation arrays
            {"1.2345e4, -23E-2]", {12345, -0.23}},

            // Whitespaces
            {"\n\r\t [\n\r\t 1\n\r\t ]\n\r\t ", {1}},
            {"[\n\r\t 1  \n\r\t ,\n\r\t -2\n\r\t ,\n\r\t 3\n\r\t ]", {1,-2,3}},
            {"[-\r\n\t 1, 2\r\n\t 3, 4abcdfghijklmnopqrstuvwxyz5]", {-1,23,45}},

            // Parse unordered array: an array where each number is a string
            {"[\"2\", \"-1\", \"3\"]", {2,-1,3}},
            {"[\"-1.0\",\"2.12314\",\"4.35\"]", {-1.0,2.12314,4.35}},

            // Minus has to be in front of number
            {"[1-,2,3]", {1,2,3}},
            {"[1\r\n\t -,2,3]", {1,2,3}},

            // Don't save empty numbers
            {"[1,,,2]", {1,2}},
            {",,,1,2]", {1,2}},
        };

        for(auto it=parse.begin(); it!=parse.end(); ++it) {
            const char* json = std::get<0>(*it);
            std::vector<double> expected_vec = std::get<1>(*it);

            CAPTURE(json);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);
            
            std::vector<double> vec;
            REQUIRE(parser.parseNumArray(vec));
            REQUIRE(vec.size() == expected_vec.size());

            for(int i=0; i<vec.size(); i++) {
                INFO("expected '" << expected_vec.at(i) << "' but got '" << vec.at(i) << "'");
                REQUIRE(std::fabs(vec.at(i)-expected_vec.at(i)) <= 0.000000000001);
            }
        }
    }

    GIVEN("invalid arrays") {
        std::vector<std::tuple<const char*, std::vector<double>>> parse {
            {"[1,-2,3", {1, -2}},
            {"[1", {}},
            {"[-", {}},
            {"[", {}},
        };

        for(auto it=parse.begin(); it!=parse.end(); ++it) {
            const char* json = std::get<0>(*it);
            std::vector<double> expected_vec = std::get<1>(*it);

            CAPTURE(json);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);
            
            std::vector<double> vec;
            REQUIRE_FALSE(parser.parseNumArray(vec));
            REQUIRE(vec.size() == expected_vec.size());

            for(int i=0; i<vec.size(); i++) {
                REQUIRE(vec.at(i) == expected_vec.at(i));
            }
        }
    }
}

SCENARIO("JsonParser::compilePath", "[compilePath]") {
    JsonParser parser;

    GIVEN("Valid paths") {
        std::vector<std::tuple<const char*, std::vector<PathSegment>>> parse = {
            {"thekey", {{"thekey"}}},
            {"obj1/thekey", {{"obj1"}, {"thekey"}}},
            {"[42]", {{42}}},

            // Escaped chars
            {"\\\"thekey", {{"\\\"thekey"}}},
            {"\\/thekey\\/", {{"/thekey/"}}},
            {"\\[]thekey\\[]", {{"[]thekey[]"}}},
            {"]\\[thekey]\\[", {{"][thekey]["}}},
            
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
            const char* path_str = std::get<0>(*it);
            std::vector<PathSegment> expected_vec = std::get<1>(*it);

            CAPTURE(path_str);

            Path path = Path(path_str);
            REQUIRE(path.size() == expected_vec.size());
            for(size_t i=0; i<path.size(); i++) {
                REQUIRE(path.at(i).type == expected_vec.at(i).type);
                if(path.at(i).type == PathSegmentType::OFFSET) {
                    REQUIRE(path.at(i).val.offset == expected_vec.at(i).val.offset);
                } else {
                    CHECK_THAT(path.at(i).val.key, Catch::Matchers::Equals(expected_vec.at(i).val.key));
                }
            }
        }
    }

    // GIVEN("Invalid paths") {
    //     std::vector<std::tuple<const char*>> parse = {
    //         {"obj1[1]obj2"}
    //     };

    //     for(auto it=parse.begin(); it!=parse.end(); ++it) {
    //         const char* path = std::get<0>(*it);

    //         INFO("path: " << path);

    //         std::vector<JsonParser::PathSegment> vec;
    //         REQUIRE(parser.compilePath(vec, path) == NULL);
    //     }
    // }
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

            CAPTURE(json);

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
            {"}", 1, ""},

            // Stops at end of object/array
                {"123}123", 1, "123"}, // Stops at }
                {"123]123", 1, "123"}, // Stops at ]
                {"\"akey\": 123}", 1, ""},

            // Skips over nested objects/arrays
                {"{\"1\": {\"1.1\": 2}} }", 1, ""},
                {"[1,[2,3]] ]", 1, ""},

            // no n-th succeding element
                {"]", 1, ""},
                {"}", 1, ""},
                {",1,2]", 3, ""},
                {", \"1\": 1, \"2\": 2}", 3, ""}
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

    std::vector<std::tuple<const char*, const int, const char*>> parse = {
            {"", 0, ""},
            {"\t\n\r ", 0, ""},
            {"\"\t\n\r \"", '\"', "\"\t\n\r \""},
            {"  ,", ',' , ","},
            {"abc", 'a' , "abc"},
            {"\t\n\r abc", 'a' , "abc"},
        };

        for(auto it = parse.begin(); it!=parse.end(); ++it) {
            const char* json = std::get<0>(*it);
            const int expectedFirstNonWhitespace = std::get<1>(*it);
            const char* json_after_exec = std::get<2>(*it);

            CAPTURE(json);

            MockStringStream stream = MockStringStream(json);
            parser.parse(&stream);
            char firstNonWhitespace = parser.skipWhitespace();
            REQUIRE(firstNonWhitespace == expectedFirstNonWhitespace);
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
}