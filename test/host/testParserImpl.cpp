#include "catch.hpp"

#include <vector>
#include <iostream>
#include <iomanip>
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

TEST_CASE("JsonParser::atEnd", "[atEnd]") {
    JsonParser parser;

    SECTION("Json not at the end of the current object/array") {
        std::vector<std::pair<const char*, const char*>> tests {         
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

            // UTF-8
            {"\"äöüÄÖÜ\"]", "\"äöüÄÖÜ\"]"},
            {"\"😀😃😄😁😆😅🤣😂🙂🙃😉😊😇\"]", "\"😀😃😄😁😆😅🤣😂🙂🙃😉😊😇\"]"},
        };

        for(int testIdx=0; testIdx<tests.size(); testIdx++) {
            const char* json = tests.at(testIdx).first;
            const char* json_after_exec = tests.at(testIdx).second;

            CAPTURE(testIdx);
            CAPTURE(json);

            MockArduino::Native::MockStringStream stream = MockArduino::Native::MockStringStream(json);
            parser.parse(stream);
            REQUIRE(parser.atEnd());
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }
    
    SECTION("Json at the end of the current object/array") {
        std::vector<std::pair<const char*, const char*>> tests {
            {"", ""},
            {" ", ""},
            {"}", "}"},
            {"  }", "}"},
            {"]", "]"},
            {"  ]", "]"}
        };

        for(int testIdx=0; testIdx<tests.size(); testIdx++) {
            const char* json = tests.at(testIdx).first;
            const char* json_after_exec = tests.at(testIdx).second;

            CAPTURE(json);

            MockArduino::Native::MockStringStream stream = MockArduino::Native::MockStringStream(json);
            parser.parse(stream);
            REQUIRE_FALSE(parser.atEnd());
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }
}

TEST_CASE("JsonParser::readString & JsonParser::skipString", "[readString, skipString]") {
    JsonParser parser;

    SECTION("valid strings") {
        std::vector<std::tuple<const char*, bool, const char*, const char*>> tests = {
            {"\"", true, "", ""},
            {"astring\", suffix", true, "astring", ", suffix"},
            {"\"astring\", suffix", false, "astring", ", suffix"},

            // Escaped chars
            {"a\\\"string\", suffix", true, "a\"string", ", suffix"},
            {"a\\\\string\", suffix", true, "a\\string", ", suffix"},
            {"a\\nstring\", suffix", true, "a\nstring", ", suffix"},

            // Skip leading whitespace
            {"\r\n\t \"astring\"\r\n\t ", false, "astring", "\r\n\t "},
            {"\r\n\t astring\"\r\n\t ", true, "\r\n\t astring", "\r\n\t "},

            // UTF-8
            {"äöüÄÖÜ\"", true, "äöüÄÖÜ", ""},
            {"😀😃😄😁😆😅🤣😂🙂🙃😉😊😇\"", true, "😀😃😄😁😆😅🤣😂🙂🙃😉😊😇", ""},

            // Read Numbers
            {"123\"", true, "123", ""},
        };

		for(int testIdx=0; testIdx<tests.size(); testIdx++) {
            const char* json = std::get<0>(tests.at(testIdx));
            const bool inStr = std::get<1>(tests.at(testIdx));
            const char* expected_str = std::get<2>(tests.at(testIdx));
            const char* json_after_exec = std::get<3>(tests.at(testIdx));

            CAPTURE(testIdx);
            CAPTURE(json);

            // readString
            MockArduino::Native::MockStringStream stream = MockArduino::Native::MockStringStream(json);
            parser.parse(stream);
            String str = "";
            REQUIRE(parser.readString(str, inStr));
            CHECK_THAT(str.c_str(), Catch::Matchers::Equals(expected_str));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));

            // skipString
            stream = MockArduino::Native::MockStringStream(json);
            parser.parse(stream);
            REQUIRE(parser.skipString(inStr));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }

    SECTION("invalid strings") {
        std::vector<std::tuple<const char*, bool, const char*, const char*>> tests = {
            {"", true, "", ""},
            {"\"", false, "", ""},
            {"astring", true, "astring", ""},
            {"\"astring", false, "astring", ""},

            // Don't read anything but a string
            {"\r\n\t }", false, "", "}"},
        };

		for(int testIdx=0; testIdx<tests.size(); testIdx++) {
            const char* json = std::get<0>(tests.at(testIdx));
            const bool inStr = std::get<1>(tests.at(testIdx));
            const char* expected_str = std::get<2>(tests.at(testIdx));
            const char* json_after_exec = std::get<3>(tests.at(testIdx));

            CAPTURE(testIdx);
            CAPTURE(json);

            // readString
            MockArduino::Native::MockStringStream stream = MockArduino::Native::MockStringStream(json);
            parser.parse(stream);
            String str = "";
            REQUIRE_FALSE(parser.readString(str, inStr));
            CHECK_THAT(str.c_str(), Catch::Matchers::Equals(expected_str));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));

            // skipString
            stream = MockArduino::Native::MockStringStream(json);
            parser.parse(stream);
            REQUIRE_FALSE(parser.skipString(inStr));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }
}

TEST_CASE("JsonParser::strcmp", "[strcmp]") {
    JsonParser parser;

    std::vector<std::tuple<const char*, const char*, bool, int, const char*>> tests = {
        // compare length
        {"astring\", suffix", "astring", true, 0, ", suffix"},
        {"\"astring\", suffix", "astring", false, 0, ", suffix"},
        {"astringlonger\", suffix", "astring", true, -1, ", suffix"},
        {"\"astringlonger\", suffix", "astring", false, -1, ", suffix"},
        {"astring\", suffix", "astringlonger", true, 1, ", suffix"},
        {"\"astring\", suffix", "astringlonger", false, 1, ", suffix"},

        // compare alphabetic order
        {"a\", suffix", "c", true, 1, ", suffix"},
        {"c\", suffix", "a", true, -1, ", suffix"},
        {"astringb\", suffix", "astringc", true, 1, ", suffix"},
        {"astringb\", suffix", "astringa", true, -1, ", suffix"},

        // no closing '"'
        {"astring", "astring", true, 1, ""},
        {"astringlonger", "astring", true, -1, ""},
        {"astring", "astringlonger", true, 1, ""},
        {"astring", "astring", true, 1, ""},

        // unescapeable char
        {"a\\b\", suffix", "ab", true, 1, ", suffix"},
        {"a\\b, suffix", "ab", true, 1, ""},
    };

		for(int testIdx=0; testIdx<tests.size(); testIdx++) {
        const char* json = std::get<0>(tests.at(testIdx));
        const char* str = std::get<1>(tests.at(testIdx));
        bool inStr = std::get<2>(tests.at(testIdx));
        int expectedResult = std::get<3>(tests.at(testIdx));
        const char* json_after_exec = std::get<4>(tests.at(testIdx));

        CAPTURE(testIdx);
        CAPTURE(json);
        CAPTURE(str);

        MockArduino::Native::MockStringStream stream = MockArduino::Native::MockStringStream(json);
        parser.parse(stream);

        REQUIRE(parser.strcmp(str, inStr) == expectedResult);
        CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
    }
}

TEST_CASE("JsonParser::parseInt") {
    JsonParser parser;

    std::vector<std::tuple<const char*, long, const char*>> tests {
        {"", 0, ""}, // No digits
        {"1", 1, ""},
        {"0", 0, ""},
        {"-1", -1, ""},
        {"1-", 1, "-"},

        // Ignore leading zeros
        {"0000123", 123, ""},
        {"-0000123", -123, ""},

        // Skip leading whitespace
        {"\r\n\t -1\r\n\t ", -1, "\r\n\t "},

        // Stop parsing at invalid chars
        {"123 3    a", 123, " 3    a"},
        {"a123", 0, "a123"},
        {"12a3", 12, "a3"},
        {"123, 456", 123, ", 456"},
    };

		for(int testIdx=0; testIdx<tests.size(); testIdx++) {
        const char* json = std::get<0>(tests.at(testIdx));
        long expected_int = std::get<1>(tests.at(testIdx));
        const char* json_after_exec = std::get<2>(tests.at(testIdx));

        CAPTURE(testIdx);
        CAPTURE(json);

        MockArduino::Native::MockStringStream stream = MockArduino::Native::MockStringStream(json);
        parser.parse(stream);

        long num = parser.parseInt();
        REQUIRE(num == expected_int);
        CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
    }
}

TEST_CASE("JsonParser::parseNum") {
    JsonParser parser;

    std::vector<std::tuple<const char*, double, const char*>> tests {
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

        // Skip leading whitespace
        {"\r\n\t -1\r\n\t ", -1, "\r\n\t "},
        {"\r\n\t -1.2", -1.2, ""},

        // Stop parsing at invalid chars
        {"123 3    a", 123, " 3    a"},
        {"a123", 0, "a123"},
        {"12a3", 12, "a3"},
        {"123, 456", 123, ", 456"},
        {"1 .2", 1.0, " .2"},
        {"1. 2", 0.0, " 2"},
    };

		for(int testIdx=0; testIdx<tests.size(); testIdx++) {
        const char* json = std::get<0>(tests.at(testIdx));
        double expected_decimal = std::get<1>(tests.at(testIdx));
        const char* json_after_exec = std::get<2>(tests.at(testIdx));
        
        CAPTURE(testIdx);
        CAPTURE(json);
        INFO("expected: " << expected_decimal);

        MockArduino::Native::MockStringStream stream = MockArduino::Native::MockStringStream(json);
        parser.parse(stream);

        double decimal = parser.parseNum();

        INFO("result: " << std::setprecision(15) << decimal);
        REQUIRE(decimal == Approx(expected_decimal));
        CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
    }
}

TEST_CASE("JsonParser::parseBool") {
    JsonParser parser;

    SECTION("valid booleans") {
        std::vector<std::tuple<const char*, bool, const char*>> tests {
            {"false", false, ""},
            {"true", true, ""},
            {"truefalse", true, "false"},
            {"falsetrue", false, "true"},

            // Skip leading whitespace
            {"\r\n\t false\r\n\t ", false, "\r\n\t "},
            {"\r\n\t true\r\n\t ", true, "\r\n\t "},
        };

		for(int testIdx=0; testIdx<tests.size(); testIdx++) {
            const char* json = std::get<0>(tests.at(testIdx));
            bool expected_bool = std::get<1>(tests.at(testIdx));
            const char* json_after_exec = std::get<2>(tests.at(testIdx));

            CAPTURE(testIdx);
            CAPTURE(json);
            INFO("expected:" << expected_bool);

            MockArduino::Native::MockStringStream stream = MockArduino::Native::MockStringStream(json);
            parser.parse(stream);

            bool result = parser.parseBool(!expected_bool);

            CAPTURE(result);

            REQUIRE(result == expected_bool);
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }

    SECTION("invalid booleans") {
        std::vector<std::tuple<const char*, const char*>> tests {
            {"f", ""},
            {"t", ""},
            {"fals", ""},
            {"tru", ""},

            {"falze", "ze"},
            {"tsue", "sue"},
        };

		for(int testIdx=0; testIdx<tests.size(); testIdx++) {
            const char* json = std::get<0>(tests.at(testIdx));
            const char* json_after_exec = std::get<1>(tests.at(testIdx));

            CAPTURE(testIdx);
            CAPTURE(json);

            MockArduino::Native::MockStringStream stream = MockArduino::Native::MockStringStream(json);
            parser.parse(stream);

            bool result = parser.parseBool();

            CAPTURE(result);

            REQUIRE_FALSE(result);
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }
}

TEST_CASE("Parse Int Array") {
    JsonParser parser;

    SECTION("valid arrays") {
        std::vector<std::tuple<const char*, bool, std::vector<long>, const char*>> tests {
            {"[-1,2,-3], suffix", false, {-1,2,-3}, ", suffix"},
            {"-1,2,-3], suffix", true, {-1,2,-3}, ", suffix"},
            {"[9268176913,-1409571945,128568915], suffix", false, {9268176913,-1409571945,128568915}, ", suffix"},

            // Whitespaces
            {"\n\r\t [\n\r\t 1\n\r\t ]\n\r\t ", false, {1}, "\n\r\t "},
            {"[\n\r\t 1  \n\r\t ,\n\r\t -2\n\r\t ,\n\r\t 3\n\r\t ]", false, {1,-2,3}, ""},
            {"[-\r\n\t 1, 2\r\n\t 3, 4abcdefghijklmnopqrstuvwxyz5]", false, {-1,23,45}, ""},

            // Parse unordered int array: an array where each integer is a string 
            {"[\"2\", \"1\", \"3\"]", false, {2,1,3}, ""},
            {"[\"2\", \"-1\", \"3\"]", false, {2,-1,3}, ""},
            {"[\"-2\", \"-1\", \"-3\"]", false, {-2,-1,-3}, ""},

            // Minus has to be in front of number
            {"[1-,2,3]", false, {1,2,3}, ""},
            {"[1\r\n\t -,2,3]", false, {1,2,3}, ""},

            // Don't save empty numbers
            {"[1,,,2], suffix", false, {1,2}, ", suffix"},
            {",,,1,2], suffix", true, {1,2}, ", suffix"},
        };

		for(int testIdx=0; testIdx<tests.size(); testIdx++) {
            const char* json = std::get<0>(tests.at(testIdx));
            bool inArray = std::get<1>(tests.at(testIdx));
            std::vector<long> expected_vec = std::get<2>(tests.at(testIdx));
            const char* json_after_exec = std::get<3>(tests.at(testIdx));

            CAPTURE(testIdx);
            CAPTURE(json);

            MockArduino::Native::MockStringStream stream = MockArduino::Native::MockStringStream(json);
            parser.parse(stream);
            
            std::vector<long> vec;
            REQUIRE(parser.parseIntArray<long>(vec, inArray));
            REQUIRE(vec.size() == expected_vec.size());

            for(int i=0; i<vec.size(); i++) {
                REQUIRE(vec.at(i) == expected_vec.at(i));
            }

            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }

    SECTION("ignore negative integers") {
        std::vector<std::tuple<const char*, bool, std::vector<long>, const char*>> tests {
            {"[1,-2,3], suffix", false, {1,3}, ", suffix"},
            {"[-1,-2,-3], suffix", false, {}, ", suffix"},
        };

        for(int testIdx=0; testIdx<tests.size(); testIdx++) {
            const char* json = std::get<0>(tests.at(testIdx));
            bool inArray = std::get<1>(tests.at(testIdx));
            std::vector<long> expected_vec = std::get<2>(tests.at(testIdx));
            const char* json_after_exec = std::get<3>(tests.at(testIdx));

            CAPTURE(testIdx);
            CAPTURE(json);

            MockArduino::Native::MockStringStream stream = MockArduino::Native::MockStringStream(json);
            parser.parse(stream);
            
            std::vector<unsigned long> vec;
            REQUIRE(parser.parseIntArray<unsigned long>(vec, inArray));
            REQUIRE(vec.size() == expected_vec.size());

            for(int i=0; i<vec.size(); i++) {
                REQUIRE(vec.at(i) == expected_vec.at(i));
            }

            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        } 
    }

    SECTION("invalid arrays") {
        std::vector<std::tuple<const char*, bool, std::vector<long>, const char*>> tests {
            {"[1,-2,3", false, {1, -2}, ""},
            {"[1", false, {}, ""},
            {"[-", false, {}, ""},
            {"[-1", false, {}, ""},
            {",suffix", false, {}, ",suffix"},
        };

		for(int testIdx=0; testIdx<tests.size(); testIdx++) {
            const char* json = std::get<0>(tests.at(testIdx));
            bool inArray = std::get<1>(tests.at(testIdx));
            std::vector<long> expected_vec = std::get<2>(tests.at(testIdx));
            const char* json_after_exec = std::get<3>(tests.at(testIdx));

            CAPTURE(testIdx);
            CAPTURE(json);

            MockArduino::Native::MockStringStream stream = MockArduino::Native::MockStringStream(json);
            parser.parse(stream);
            
            std::vector<long> vec;
            REQUIRE_FALSE(parser.parseIntArray<long>(vec, inArray));
            REQUIRE(vec.size() == expected_vec.size());

            for(int i=0; i<vec.size(); i++) {
                REQUIRE(vec.at(i) == expected_vec.at(i));
            }

            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }
}

TEST_CASE("Parse Num Array") {
    JsonParser parser;

    SECTION("valid arrays") {
        std::vector<std::tuple<const char*, bool, std::vector<double>, const char*>> tests {
            // Int arrays
            {"[1,-2,3]", false, {1,-2,3}, ""},
            {"[-1,2,-3]", false, {-1,2,-3}, ""},
            {"[9268176913,-1409571945,128568915]", false, {9268176913,-1409571945,128568915}, ""},

            // Decimal arrays
            {"[-1.0,2.12314,4.35]", false, {-1.0,2.12314,4.35}, ""},
            {"-1.0,2.12314,4.35]", true, {-1.0,2.12314,4.35}, ""},

            // Scientific notation arrays
            {"1.2345e4, -23E-2]", true, {12345, -0.23}, ""},

            // Whitespaces
            {"\n\r\t [\n\r\t 1\n\r\t ]\n\r\t ", false, {1}, "\n\r\t "},
            {"[\n\r\t 1  \n\r\t ,\n\r\t -2\n\r\t ,\n\r\t 3\n\r\t ]", false, {1,-2,3}, ""},
            {"[-\r\n\t 1, 2\r\n\t 3, 4abcdfghijklmnopqrstuvwxyz5]", false, {-1,23,45}, ""},

            // Parse unordered array: an array where each number is a string
            {"[\"2\", \"-1\", \"3\"]", false, {2,-1,3}, ""},
            {"[\"-1.0\",\"2.12314\",\"4.35\"]", false, {-1.0,2.12314,4.35}, ""},

            // Minus has to be in front of number
            {"[1-,2,3]", false, {1,2,3}, ""},
            {"[1\r\n\t -,2,3]", false, {1,2,3}, ""},

            // Don't save empty numbers
            {"[1,,,2]", false, {1,2}, ""},
            {",,,1,2]", true, {1,2}, ""},

            // Only read the array from stream
            {"[1,2,3], suffix", false, {1,2,3}, ", suffix"}
        };

		for(int testIdx=0; testIdx<tests.size(); testIdx++) {
            const char* json = std::get<0>(tests.at(testIdx));
            bool inArray = std::get<1>(tests.at(testIdx));
            std::vector<double> expected_vec = std::get<2>(tests.at(testIdx));
            const char* json_after_exec = std::get<3>(tests.at(testIdx));

            CAPTURE(testIdx);
            CAPTURE(json);

            MockArduino::Native::MockStringStream stream = MockArduino::Native::MockStringStream(json);
            parser.parse(stream);
            
            std::vector<double> vec;
            REQUIRE(parser.parseNumArray(vec, inArray));
            REQUIRE(vec.size() == expected_vec.size());

            for(int i=0; i<vec.size(); i++) {
                INFO("expected '" << std::setprecision(30) << expected_vec.at(i) << "' but got '" << vec.at(i) << "'");
                REQUIRE(vec.at(i) == Approx(expected_vec.at(i)));
            }

            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }

    SECTION("invalid arrays") {
        std::vector<std::tuple<const char*, bool, std::vector<double>, const char*>> tests {
            {"[1,-2,3", false, {1, -2}, ""},
            {"[1", false, {}, ""},
            {"[-", false, {}, ""},
            {"[", false, {}, ""},

            // Incorrect parameter 'inArray'
            {"1,2,3]", false, {}, "1,2,3]"},

            // Only read the array from stream
            {"\"akey\": 123", false, {}, "\"akey\": 123"},
            {"1,2,3], suffix", false, {}, "1,2,3], suffix"},
        };

		for(int testIdx=0; testIdx<tests.size(); testIdx++) {
            const char* json = std::get<0>(tests.at(testIdx));
            bool inArray = std::get<1>(tests.at(testIdx));
            std::vector<double> expected_vec = std::get<2>(tests.at(testIdx));
            const char* json_after_exec = std::get<3>(tests.at(testIdx));

            CAPTURE(testIdx);
            CAPTURE(json);

            MockArduino::Native::MockStringStream stream = MockArduino::Native::MockStringStream(json);
            parser.parse(stream);
            
            std::vector<double> vec;
            REQUIRE_FALSE(parser.parseNumArray(vec, inArray));
            REQUIRE(vec.size() == expected_vec.size());

            for(int i=0; i<vec.size(); i++) {
                REQUIRE(vec.at(i) == expected_vec.at(i));
            }

            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }
}

TEST_CASE("JsonParser::compilePath", "[compilePath]") {
    JsonParser parser;

    SECTION("Valid paths") {
        std::vector<std::tuple<const char*, std::vector<PathSegment>>> tests {
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

            // UTF-8
            {"ä/ö/ü", {{"ä"}, {"ö"}, {"ü"}}},
            {"😀😃😄/😁😆😅/🤣😂🙂", {{"😀😃😄"}, {"😁😆😅"}, {"🤣😂🙂"}}},
        };

        for(int testIdx=0; testIdx<tests.size(); testIdx++) {   
            const char* path_str = std::get<0>(tests.at(testIdx));
            std::vector<PathSegment> expected_vec = std::get<1>(tests.at(testIdx));

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

    // SECTION("Invalid paths") {
    //     std::vector<std::tuple<const char*>> tests {
    //         {"obj1[1]obj2"}
    //     };

    //     for(auto it=parse.begin(); it!=parse.end(); ++it) {
    //         const char* path = std::get<0>(tests.at(testIdx));

    //         INFO("path: " << path);

    //         std::vector<JsonParser::PathSegment> vec;
    //         REQUIRE(parser.compilePath(vec, path) == NULL);
    //     }
    // }
}

/////////////////////
// Private methods //
/////////////////////

TEST_CASE("JsonParser::next", "[private, next]") {
    JsonParser parser;

    SECTION("Json with next") {
        std::vector<std::tuple<const char*, size_t, const char*>> tests {
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
            {",\"\\\"akey\\\": 123\": 123}", 1, "\"\\\"akey\\\": 123\": 123}"},

            // UTF-8
            {"äöüÄÖÜ, 1", 1, "1"},
            {"😀😃😄😁😆😅🤣😂🙂🙃😉😊😇, 1", 1, "1"},
        };

		for(int testIdx=0; testIdx<tests.size(); testIdx++) {
            const char* json = std::get<0>(tests.at(testIdx));
            size_t n = std::get<1>(tests.at(testIdx));
            const char* json_after_exec = std::get<2>(tests.at(testIdx));

            CAPTURE(testIdx);
            CAPTURE(json);

            MockArduino::Native::MockStringStream stream = MockArduino::Native::MockStringStream(json);
            parser.parse(stream);
            REQUIRE(parser.next(n));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }

    SECTION("Json without next") {
        std::vector<std::tuple<const char*, size_t, const char*>> tests {
            {"", 1, ""},
            {"123", 1, ""},

            // Don't exit current object/array
            {"}", 1, "}"},
            {"]", 1, "]"},
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

		for(int testIdx=0; testIdx<tests.size(); testIdx++) {
            const char* json = std::get<0>(tests.at(testIdx));
            size_t n = std::get<1>(tests.at(testIdx));
            const char* json_after_exec = std::get<2>(tests.at(testIdx));

            CAPTURE(testIdx);
            CAPTURE(json);

            MockArduino::Native::MockStringStream stream = MockArduino::Native::MockStringStream(json);
            parser.parse(stream);
            REQUIRE_FALSE(parser.next(n));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }
}

TEST_CASE("JsonParser::skipWhitespace", "[private, skipWhitespace]") {
    JsonParser parser;

    std::vector<std::tuple<const char*, const int, std::string>> tests {
            {"", 0, ""},
            {"\t\n\r ", 0, ""},
            {"\"\t\n\r \"", '\"', "\"\t\n\r \""},
            {"  ,", ',' , ","},
            {"abc", 'a' , "abc"},
            {"\t\n\r abc", 'a' , "abc"},

            // UTF-8
            {"\r\n\t äöüÄÖÜ", ((std::string)"ä")[0], "äöüÄÖÜ"},
            {"\r\n\t 😀😃😄", ((std::string)"😀")[0], "😀😃😄"},
        };

        for(int testIdx=0; testIdx<tests.size(); testIdx++) {
            const char* json = std::get<0>(tests.at(testIdx));
            const int expectedFirstNonWhitespace = std::get<1>(tests.at(testIdx));
            const char* json_after_exec = std::get<2>(tests.at(testIdx)).c_str();

            CAPTURE(testIdx);
            CAPTURE(json);

            MockArduino::Native::MockStringStream stream = MockArduino::Native::MockStringStream(json);
            parser.parse(stream);
            char firstNonWhitespace = parser.skipWhitespace();
            REQUIRE(firstNonWhitespace == expectedFirstNonWhitespace);
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
}