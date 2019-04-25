#include "catch.hpp"

#include <vector>
#include <iostream>
#include <iomanip>
#include <utility>
#include <cstring>
#include <sstream>
#include <cmath>

#include <Arduino.h>
#include <MockStream.h>

#define protected public
#define private   public
#include <JsonParser.h>
#undef protected
#undef private

#include <Path.h>

using namespace JStream;

TEST_CASE("JsonParser::nextVal", "[nextVal]") {
    JsonParser parser;

    SECTION("another value exists") {
        std::vector<std::pair<const char*, const char*>> tests {
            {",1,2]", "1,2]"},
            {", 1,2]", "1,2]"},

            // Skip over current value
            {"1,2]", "2]"},
            {"12345,2]", "2]"},
            {"[1,2,3], 2]", "2]"},
            {"{\"1\": 1}, 2]", "2]"},
            {"[[[[[]]]]],2]", "2]"},
            {"{{{{{}}}}},2]", "2]"},

            // Skip whitespace after ','
            {"1, 2]", "2]"},
            {"1, 2", "2"},

            // UTF-8
            {",\"äöüÄÖÜ\"]", "\"äöüÄÖÜ\"]"},
            {",\"😀😃😄😁😆😅🤣😂🙂🙃😉😊😇\"]", "\"😀😃😄😁😆😅🤣😂🙂🙃😉😊😇\"]"},
        };

        for(unsigned int testIdx=0; testIdx<tests.size(); testIdx++) {
            const char* json = tests.at(testIdx).first;
            const char* json_after_exec = tests.at(testIdx).second;

            CAPTURE(json);

            ArduinoTestUtils::MockStream stream = ArduinoTestUtils::MockStream(json);
            parser.parse(stream);
            REQUIRE(parser.nextVal());
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }
    
    SECTION("no more values exists") {
        std::vector<std::pair<const char*, const char*>> tests {
            // Don't read closing ']'/'}'
            {"]", "]"},
            {"}", "}"},

            // Skip current value
            {"123]", "]"},
            {"123}", "}"},
            {"\"astring\"]", "]"},
            {"\"astring\"}", "}"},
            {"[[[]]]]", "]"},
            {"[[[]]]}", "}"},
            {"{{{}}}]", "]"},
            {"{{{}}}}", "}"},
        };

        for(unsigned int testIdx=0; testIdx<tests.size(); testIdx++) {
            const char* json = tests.at(testIdx).first;
            const char* json_after_exec = tests.at(testIdx).second;

            CAPTURE(testIdx);
            CAPTURE(json);

            ArduinoTestUtils::MockStream stream = ArduinoTestUtils::MockStream(json);
            parser.parse(stream);
            REQUIRE_FALSE(parser.nextVal());
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }
}

TEST_CASE("JsonParser::nextKey", "[nextKey]") {
    JsonParser parser;

    SECTION("Next Key exists") {
        // Json string | result of 'nextKey' | resulting Json string
        std::vector<std::tuple<const char*, const char*, const char*>> tests {
            {"\"akey\": 123}", "akey", "123}"},
            {"\n\r\t \"akey\": 123}", "akey", "123}"},
            {",\"akey\": 123}", "akey", "123}"},

            // Read escpaped chars
            {",\"\\\"akey\\\": 123\": 123}", "\"akey\": 123", "123}"},

            // Skip invalid key
            {", \"invalidkey\" 123, \"akey\": 123}", "akey", "123}"},
            {"\"invalidkey\" 123, \"akey\": 123}", "akey", "123}"},
            {", 1, 2, 3, \"akey\": 123}", "akey", "123}"},
            {", [1,2,3], {\"key1\": 123}, \"akey\": 123}", "akey", "123}"},

            // Skip current value
            {"[[[]]], \"akey\": 123}", "akey", "123}"},
            {"{{{}}}, \"akey\": 123}", "akey", "123}"},

            // UTF-8
            {",\"äöüÄÖÜ\": 1]", "äöüÄÖÜ", "1]"},
            {",\"😀😃😄😁😆😅🤣😂🙂🙃😉😊😇\": 1]", "😀😃😄😁😆😅🤣😂🙂🙃😉😊😇", "1]"},
        };

        for(unsigned int testIdx=0; testIdx<tests.size(); testIdx++) {
            const char* json = std::get<0>(tests.at(testIdx));
            const char* expected_key = std::get<1>(tests.at(testIdx));
            const char* json_after_exec = std::get<2>(tests.at(testIdx));

            CAPTURE(testIdx);
            CAPTURE(json);

            // Test capturing the key
            ArduinoTestUtils::MockStream stream = ArduinoTestUtils::MockStream(json);
            parser.parse(stream);
            String key = "";
            REQUIRE(parser.nextKey(&key));
            CHECK_THAT(key.c_str(), Catch::Matchers::Equals(expected_key));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));

            // Test not capturing the key
            stream = ArduinoTestUtils::MockStream(json);
            parser.parse(stream);
            REQUIRE(parser.nextKey(nullptr));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }

    SECTION("No next Key exists") {
        // Json string | result of 'nextKey' | resulting Json string
        std::vector<std::tuple<const char*, const char*>> tests {
            {"", ""},
            {"123", ""},

            // Don't exit current object/array
            {"123}123", "}123"},
            {"123]123", "]123"},
            {"\n\r\t }, 123", "}, 123"},
            {"\n\r\t ], 123", "], 123"},

            // Skips malformed keys
            {",\"akey\" 123", ""},
            {"\"akey\" 123", ""},

            // Skip until end in arrays
            {", 1, 2, 3", ""},
            {", 1, 2, 3", ""}
        };

        for(unsigned int testIdx=0; testIdx<tests.size(); testIdx++) {
            const char* json = std::get<0>(tests.at(testIdx));
            const char* json_after_exec = std::get<1>(tests.at(testIdx));

            CAPTURE(testIdx);
            CAPTURE(json);

            // Test capturing the key
            ArduinoTestUtils::MockStream stream = ArduinoTestUtils::MockStream(json);
            parser.parse(stream);
            String key = "";
            REQUIRE_FALSE(parser.nextKey(&key));
            CAPTURE(key);
            REQUIRE(key == "");
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));

            // Test not capturing the key
            stream = ArduinoTestUtils::MockStream(json);
            parser.parse(stream);
            REQUIRE_FALSE(parser.nextKey(nullptr));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }
}

TEST_CASE("JsonParser::findKey", "[findKey]") {
    JsonParser parser;
    SECTION("Json with matching key") {
        // Json | key | resulting Json
        std::vector<std::tuple<const char*, const char*, const char*>> tests {
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
            {"\n\t\r \"thekey\": 123, \"akey\": 312}", "thekey", "123, \"akey\": 312}"},

            // UTF-8
            {"\"akey\": 1, \"äöüÄÖÜ\": 2}", "äöüÄÖÜ", "2}"},
            {"\"akey\": 1, \"😀😃😄😁😆😅🤣😂🙂🙃😉😊😇\": 2}", "😀😃😄😁😆😅🤣😂🙂🙃😉😊😇", "2}"},
        };
        
        for(unsigned int testIdx=0; testIdx<tests.size(); testIdx++) {
            const char* json = std::get<0>(tests.at(testIdx));
            const char* thekey = std::get<1>(tests.at(testIdx));
            const char* json_after_exec = std::get<2>(tests.at(testIdx));

            CAPTURE(testIdx);
            CAPTURE(json);
            CAPTURE(thekey);  

            ArduinoTestUtils::MockStream stream = ArduinoTestUtils::MockStream(json);
            parser.parse(stream);
            bool success = parser.findKey(thekey);
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
            REQUIRE(success);
        }
    }

    SECTION("Json without matching key") {
        // Json | key | resulting Json
        std::vector<std::tuple<const char*, const char*, const char*>> tests {
            {"", "", ""},
            {"", "thekey", ""},

            // Don't exit current object/array
            {"}, suffix", "thekey", "}, suffix"},
            {"], suffix", "thekey", "], suffix"},

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
            {", \"thekey\"}", "thekey", "}"},

            // UTF-8
            {", \"äöüÄÖ\": 1}", "äöüÄÖÜ", "}"},
            {", \"😀😃😄😁😆😅🤣😂🙂🙃😉😊\": 1}", "😀😃😄😁😆😅🤣😂🙂🙃😉😊😇", "}"},
        };
        
        for(unsigned int testIdx=0; testIdx<tests.size(); testIdx++) {
            const char* json = std::get<0>(tests.at(testIdx));
            const char* thekey = std::get<1>(tests.at(testIdx));
            const char* json_after_exec = std::get<2>(tests.at(testIdx));

            CAPTURE(testIdx);
            CAPTURE(json);
            CAPTURE(thekey);

            ArduinoTestUtils::MockStream stream = ArduinoTestUtils::MockStream(json);
            parser.parse(stream);
            bool success = parser.findKey(thekey);
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
            REQUIRE_FALSE(success);
        }
    }
}

TEST_CASE("JsonParser::enterArr && ::enterObj", "[enterCollection, enterArr, enterObj]") {
    JsonParser parser;

    SECTION("Either ::enterArr or ::enterObj successfull") {
        std::vector<std::tuple<const char*, char, const char*>> tests {
            {"[, suffix", '[', ", suffix"},
            {"{, suffix", '{', ", suffix"},
            {"\r\n\t [", '[', ""},
            {"\r\n\t {", '{', ""},
        };

		for(unsigned int testIdx=0; testIdx<tests.size(); testIdx++) {
            const char* json = std::get<0>(tests.at(testIdx));
            char collectionType = std::get<1>(tests.at(testIdx));
            const char* json_after_exec = std::get<2>(tests.at(testIdx));

            CAPTURE(testIdx);
            CAPTURE(json);

            // enterArr
            ArduinoTestUtils::MockStream stream = ArduinoTestUtils::MockStream(json);
            parser.parse(stream);
            REQUIRE(parser.enterArr() == (collectionType == '['));
            if(collectionType == '[') CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
            else CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals((String(collectionType) + json_after_exec).c_str()));

            // enterObj
            stream = ArduinoTestUtils::MockStream(json);
            parser.parse(stream);
            REQUIRE(parser.enterObj() == (collectionType == '{'));
            if(collectionType == '{') CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
            else CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals((String(collectionType) + json_after_exec).c_str()));
        }
    }
    
    SECTION("Neither is successfull") {
        std::vector<std::tuple<const char*, const char*>> tests {
            {"", ""},
            {"\r\n\t ", ""},
            {"\"akey\": []", "\"akey\": []"},
            {": []", ": []"},
            {"123 [", "123 ["},
        };

		for(unsigned int testIdx=0; testIdx<tests.size(); testIdx++) {
            const char* json = std::get<0>(tests.at(testIdx));
            const char* json_after_exec = std::get<1>(tests.at(testIdx));

            CAPTURE(testIdx);
            CAPTURE(json);

            // enterArr
            ArduinoTestUtils::MockStream stream = ArduinoTestUtils::MockStream(json);
            parser.parse(stream);
            REQUIRE_FALSE(parser.enterArr());
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));

            // enterObj
            stream = ArduinoTestUtils::MockStream(json);
            parser.parse(stream);
            REQUIRE_FALSE(parser.enterObj());
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }
}

TEST_CASE("JsonParser::exitCollection" , "[exitCollection]") {
    JsonParser parser;

    SECTION("Successfull exits") {
        std::vector<std::tuple<const char*, size_t, const char*>> tests {
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
            {",{1,2,3},{7,8,9}], suffix", 1, ", suffix"},

            // UTF-8
            {",\"äöüÄÖÜ\": 1], suffix", 1, ", suffix"},
            {",\"😀😃😄😁😆😅🤣😂🙂🙃😉😊😇\": 1], suffix", 1, ", suffix"},
            {",😀😃😄😁😆😅🤣😂🙂🙃😉😊😇], suffix", 1, ", suffix"},
        };

		for(unsigned int testIdx=0; testIdx<tests.size(); testIdx++) {
            const char* json = std::get<0>(tests.at(testIdx));
            size_t levels = std::get<1>(tests.at(testIdx));
            const char* json_after_exec = std::get<2>(tests.at(testIdx));

            CAPTURE(testIdx);
            CAPTURE(json);

            ArduinoTestUtils::MockStream stream = ArduinoTestUtils::MockStream(json);
            parser.parse(stream);
            REQUIRE(parser.exitCollection(levels));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }

    SECTION("Unsuccessfull exits") {
        std::vector<std::tuple<const char*, size_t, const char*>> tests {
            // Empty Json
            {"", 1, ""},
            {"", 2, ""},
            {"", 5, ""},

            // Ignore strings
            {"\"a String ]\"", 1, ""}
        };

		for(unsigned int testIdx=0; testIdx<tests.size(); testIdx++) {
            const char* json = std::get<0>(tests.at(testIdx));
            size_t levels = std::get<1>(tests.at(testIdx));
            const char* json_after_exec = std::get<2>(tests.at(testIdx));

            CAPTURE(testIdx);
            CAPTURE(json);

            ArduinoTestUtils::MockStream stream = ArduinoTestUtils::MockStream(json);
            parser.parse(stream);
            REQUIRE_FALSE(parser.exitCollection(levels));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }
}

TEST_CASE("JsonParser::skipCollection", "[skipCollection]") {
    JsonParser parser;

    SECTION("Successfull skips") {
        std::vector<std::tuple<const char*, const char*>> tests = {
            {"[], suffix", ", suffix"},
            {"{}, suffix", ", suffix"},

            // Whitespaces
            {"\r\n\t [\r\n\t ], suffix", ", suffix"},
            {"\r\n\t {\r\n\t }, suffix", ", suffix"},

            // Skip nested objects/arrays
            {"{[1,2,3],[7,8,9]}, suffix", ", suffix"},
            {"{{1,2,3},{7,8,9}}, suffix", ", suffix"},
            {"[[1,2,3],[7,8,9]], suffix", ", suffix"},
            {"[{1,2,3},{7,8,9}], suffix", ", suffix"},

            // UTF-8
            {"[äöüÄÖÜ], suffix", ", suffix"},
            {"[😀😃😄😁😆😅🤣😂🙂🙃😉😊😇], suffix", ", suffix"},
        };

		for(unsigned int testIdx=0; testIdx<tests.size(); testIdx++) {
            const char* json = std::get<0>(tests.at(testIdx));
            const char* json_after_exec = std::get<1>(tests.at(testIdx));

            CAPTURE(testIdx);
            CAPTURE(json);

            ArduinoTestUtils::MockStream stream = ArduinoTestUtils::MockStream(json);
            parser.parse(stream);
            REQUIRE(parser.skipCollection());
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }
}

TEST_CASE("JsonParser::find", "[find]") {
    JsonParser parser;

    SECTION("Existing path") {
        std::vector<std::tuple<const char*, const char*, const char*>> tests = {
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

            // UTF-8
            {"\"obj1\": {\"äöüÄÖÜ\": 1}}", "obj1/äöüÄÖÜ", "1}}"},
            {"\"obj1\": {\"😀😃😄😁😆😅🤣😂🙂🙃😉😊😇\": 1}}", "obj1/😀😃😄😁😆😅🤣😂🙂🙃😉😊😇", "1}}"},
        };

		for(unsigned int testIdx=0; testIdx<tests.size(); testIdx++) {
            const char* json = std::get<0>(tests.at(testIdx));
            const char* path_str = std::get<1>(tests.at(testIdx));
            const char* json_after_exec = std::get<2>(tests.at(testIdx));

            CAPTURE(testIdx);
            CAPTURE(json);
            CAPTURE(path_str);

            ArduinoTestUtils::MockStream stream = ArduinoTestUtils::MockStream(json);
            parser.parse(stream);
            Path path = Path(path_str);
            REQUIRE(parser.find(path));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));

            stream = ArduinoTestUtils::MockStream(json);
            parser.parse(stream);
            REQUIRE(parser.find(path_str));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }

    SECTION("Non existing path") {
        std::vector<std::tuple<const char*, const char*, const char*>> tests = {
            // Index out of bounds
            {"0, 1, 2, 3, 4]", "[5]", "]"},
            {"[1,2,3], [3,4,5], [6,7,8]]", "[1][3]", "], [6,7,8]]"},
        };

		for(unsigned int testIdx=0; testIdx<tests.size(); testIdx++) {
            const char* json = std::get<0>(tests.at(testIdx));
            const char* path_str = std::get<1>(tests.at(testIdx));
            const char* json_after_exec = std::get<2>(tests.at(testIdx));

            CAPTURE(testIdx);
            CAPTURE(json);
            CAPTURE(path_str);

            ArduinoTestUtils::MockStream stream = ArduinoTestUtils::MockStream(json);
            parser.parse(stream);

            Path path = Path(path_str);
            REQUIRE_FALSE(parser.find(path));
            CHECK_THAT(stream.readString().c_str(), Catch::Matchers::Equals(json_after_exec));
        }
    }
}