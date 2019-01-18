#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <iostream>
#include <utility>
#include <vector>
#include <tuple>

#include <Arduino.h>
#include <JsonTokenizer.h>
#include <Streams/StringStream.h>

using namespace JStream;

std::ostream& operator << ( std::ostream& os, JsonTokenizer::Token const& value ) {
    os << JsonTokenizer::tokenTypeToStr(value);
    return os;
}

std::ostream& operator << ( std::ostream& os, JsonTokenizer::ParseError const& value ) {
    os << JsonTokenizer::errorToStr(value);
    return os;
}

void matchGeneratedTokens(JsonTokenizer* tok, String json, std::vector<std::pair<JsonTokenizer::Token, String>> tokens) {
    // Don't check Token Values
    tok->tokenize(json);
    for(int k=0; k<tokens.size(); k++) {
        REQUIRE(tok->next(nullptr) == tokens[k].first);
    }
    REQUIRE_FALSE(tok->hasNext());

    // Check Token Values
    tok->tokenize(json);
    for(int k=0; k<tokens.size(); k++) {
        String buf = "";
        REQUIRE(tok->next(&buf) == tokens[k].first);
        REQUIRE(buf == tokens[k].second);
    }
    REQUIRE_FALSE(tok->hasNext());
}

void matchGeneratedTokensAndErrors(JsonTokenizer* tok, String json, std::vector<std::tuple<JsonTokenizer::Token, JsonTokenizer::ParseError, String>> tokens) {
    // Don't check Token Values
    tok->tokenize(json);
    for(int k=0; k<tokens.size(); k++) {
        REQUIRE(tok->next(nullptr) == std::get<0>(tokens[k]));
        if(std::get<0>(tokens[k]) == JsonTokenizer::Token::ERR) {
            REQUIRE(tok->getErrorCode() == std::get<1>(tokens[k]));
        }
    }
    REQUIRE_FALSE(tok->hasNext());

    // Check Token Values
    tok->tokenize(json);
    for(int k=0; k<tokens.size(); k++) {
        String buf = "";
        REQUIRE(tok->next(&buf) == std::get<0>(tokens[k]));
        if(std::get<0>(tokens[k]) == JsonTokenizer::Token::ERR) {
            REQUIRE(tok->getErrorCode() == std::get<1>(tokens[k]));
        }

        REQUIRE(buf == std::get<2>(tokens[k]));
    }
    REQUIRE_FALSE(tok->hasNext());
}

static String jsonExample1 = 
    "{ \
        \"string2\": \"Nightman\", \
        \"multiline\": \"I \nam a\nmultiline String\", \
        \"link\": \"https://de.wikipedia.org/wiki/Arduino_(Plattform)\", \
        \"email\": \"ardu@ino.com\", \
        \"quoted_str\" : \"\\\"I am a quote\\\"\", \
        \"int\": 198, \
        \"double\": -2.346326, \
        \"large_num\": 0.124E-123, \
        \"false\": false, \
        \"true\": true, \
        \"null\": null \
    }";

/**
 * Test if the Tokenizer tokenizes the basic building block Tokens (everything except a String, Field Name or a Number) correctly
 */
SCENARIO("Tokenize basic Tokens") {
    auto tok = JsonTokenizer();
    std::vector<std::pair<String, JsonTokenizer::Token>> tokens = 
        {{"{", JsonTokenizer::Token::OBJ_START},{"}", JsonTokenizer::Token::OBJ_END},{"[", JsonTokenizer::Token::ARR_START},{"]", JsonTokenizer::Token::ARR_END}, 
        {",", JsonTokenizer::Token::COMMA}, {"null", JsonTokenizer::Token::KW_NULL}, {"true", JsonTokenizer::Token::KW_TRUE}, 
        {"false", JsonTokenizer::Token::KW_FALSE}};
    
    for(auto it=tokens.begin(); it!=tokens.end(); ++it) {
        tok.tokenize(new StringStream(it->first));
        REQUIRE(tok.hasNext());
        String buf = "";
        REQUIRE(tok.next(&buf) == it->second);
        REQUIRE(buf == "");
        REQUIRE_FALSE(tok.hasNext());
    }
}

SCENARIO("Tokenize numbers", "[tokenize]") {
    auto tok = JsonTokenizer();

    // INTEGERS
    GIVEN("valid integers") {
        std::vector<std::pair<String, std::vector<std::pair<JsonTokenizer::Token, String>>>> ints = {
            {"0", {{JsonTokenizer::Token::NUM, "0"}}},
            {"-0", {{JsonTokenizer::Token::NUM, "-0"}}},
            {"2834719075129051986198309182370", {{JsonTokenizer::Token::NUM, "2834719075129051986198309182370"}}},
            {"-4192359128129591469102380120350", {{JsonTokenizer::Token::NUM, "-4192359128129591469102380120350"}}}
        };

        for(auto it=ints.begin(); it!=ints.end(); ++it) {
            matchGeneratedTokens(&tok, it->first, it->second);
        }
    }
    GIVEN("invalid integers") {
        std::vector<std::pair<String, std::vector<std::tuple<JsonTokenizer::Token, JsonTokenizer::ParseError, String>>>> ints = {
            {"-", {{JsonTokenizer::Token::ERR, JsonTokenizer::ParseError::MALFORMED_NUM, "-"}}},
            {"0234", {{JsonTokenizer::Token::ERR, JsonTokenizer::ParseError::MALFORMED_NUM, "0234"}}},
            {"-0234", {{JsonTokenizer::Token::ERR, JsonTokenizer::ParseError::MALFORMED_NUM, "-0234"}}}
        };

        for(auto it=ints.begin(); it!=ints.end(); ++it) {
            matchGeneratedTokensAndErrors(&tok, it->first, it->second);
        }
    }

    // FRACTIONS
    GIVEN("valid decimals") {
        std::vector<std::pair<String, std::vector<std::pair<JsonTokenizer::Token, String>>>> fracs = {
            {"0.0", {{JsonTokenizer::Token::NUM, "0.0"}}},
            {"0.6343", {{JsonTokenizer::Token::NUM, "0.6343"}}},
            {"0.0006343000", {{JsonTokenizer::Token::NUM, "0.0006343000"}}}
        };

        for(auto it=fracs.begin(); it!=fracs.end(); ++it) {
            matchGeneratedTokens(&tok, it->first, it->second);
        }
    }
    GIVEN("invalid decimals") {
        std::vector<std::pair<String, std::vector<std::tuple<JsonTokenizer::Token, JsonTokenizer::ParseError, String>>>> fracs = {
            {".", {{JsonTokenizer::Token::ERR, JsonTokenizer::ParseError::UNQUOTED_STR, "."}}},
            {"0.a", {{JsonTokenizer::Token::ERR, JsonTokenizer::ParseError::MALFORMED_NUM, "0."}, 
                    {JsonTokenizer::Token::ERR, JsonTokenizer::ParseError::UNQUOTED_STR, "a"}}},
            {"0.012.032", {{JsonTokenizer::Token::ERR, JsonTokenizer::ParseError::MALFORMED_NUM, "0.012.032"}}}
        };

        for(auto it=fracs.begin(); it!=fracs.end(); ++it) {
            matchGeneratedTokensAndErrors(&tok, it->first, it->second);
        }
    }

    // EXPONENTS
    GIVEN("valid exponentes") {
        std::vector<std::pair<String, std::vector<std::pair<JsonTokenizer::Token, String>>>> exps = {
            {"0e10", {{JsonTokenizer::Token::NUM, "0e10"}}},
            {"0e0", {{JsonTokenizer::Token::NUM, "0e0"}}},
            {"0e00001", {{JsonTokenizer::Token::NUM, "0e00001"}}},
            {"0E+311", {{JsonTokenizer::Token::NUM, "0E+311"}}}
        };

        for(auto it=exps.begin(); it!=exps.end(); ++it) {
            matchGeneratedTokens(&tok, it->first, it->second);
        }
    }
    GIVEN("invalid exponents") {
        std::vector<std::pair<String, std::vector<std::tuple<JsonTokenizer::Token, JsonTokenizer::ParseError, String>>>> exps = {
            {"e", {{JsonTokenizer::Token::ERR, JsonTokenizer::ParseError::UNQUOTED_STR, "e"}}},
            {"E", {{JsonTokenizer::Token::ERR, JsonTokenizer::ParseError::UNQUOTED_STR, "E"}}},
            {"0e+", {{JsonTokenizer::Token::ERR, JsonTokenizer::ParseError::MALFORMED_NUM, "0e+"}}},
            {"0e-", {{JsonTokenizer::Token::ERR, JsonTokenizer::ParseError::MALFORMED_NUM, "0e-"}}},
            {"0e--", {{JsonTokenizer::Token::ERR, JsonTokenizer::ParseError::MALFORMED_NUM, "0e--"}}},
            {"0e++", {{JsonTokenizer::Token::ERR, JsonTokenizer::ParseError::MALFORMED_NUM, "0e++"}}}
        };

        for(auto it=exps.begin(); it!=exps.end(); ++it) {
            matchGeneratedTokensAndErrors(&tok, it->first, it->second);
        }
    }

    // NUMBERS
    GIVEN("valid numbers") {
        std::vector<std::pair<String, std::vector<std::pair<JsonTokenizer::Token, String>>>> nums = {
            {"0.1", {{JsonTokenizer::Token::NUM, "0.1"}}},
            {"-0.1", {{JsonTokenizer::Token::NUM, "-0.1"}}},
            {"-0.1", {{JsonTokenizer::Token::NUM, "-0.1"}}},
            {"0e10", {{JsonTokenizer::Token::NUM, "0e10"}}},
            {"-0.1E+32", {{JsonTokenizer::Token::NUM, "-0.1E+32"}}}
        };

        for(int i=0; i<nums.size(); i++) {
            matchGeneratedTokens(&tok, nums[i].first, nums[i].second);
        }
    }
    GIVEN("invalid numbers") {
        std::vector<std::pair<String, std::vector<std::tuple<JsonTokenizer::Token, JsonTokenizer::ParseError, String>>>> nums = {
            {"0.1.10", {{JsonTokenizer::Token::ERR, JsonTokenizer::ParseError::MALFORMED_NUM, "0.1.10"}}},
            {"0e-12e+2", {{JsonTokenizer::Token::ERR, JsonTokenizer::ParseError::MALFORMED_NUM, "0e-12e+2"}}}
        };

        for(auto it=nums.begin(); it!=nums.end(); ++it) {
            matchGeneratedTokensAndErrors(&tok, it->first, it->second);
        }
    }
}

SCENARIO("Tokenize strings", "[tokenize]") {
    auto tok = JsonTokenizer();
    GIVEN("valid strings") {
        std::vector<std::pair<String, std::vector<std::pair<JsonTokenizer::Token, String>>>> strs = {
            {"\"\"", {{JsonTokenizer::Token::STR, ""}}},
            {"\"\\\"\"", {{JsonTokenizer::Token::STR, "\""}}}, // Escape '"'
            {"\"\\n\"", {{JsonTokenizer::Token::STR, "\n"}}},  // Escape '\n'
            {"\"\\t\"", {{JsonTokenizer::Token::STR, "\t"}}},  // Escape '\t'
            {"\"\\r\"", {{JsonTokenizer::Token::STR, "\r"}}},  // Escape '\r'
            {"\"\\b\"", {{JsonTokenizer::Token::STR, "\b"}}},  // Escape '\b'
            {"\"\\f\"", {{JsonTokenizer::Token::STR, "\f"}}},  // Escape '\f'
            {"\"\\\\\"", {{JsonTokenizer::Token::STR, "\\"}}}  // Escape '\'
        };

        for(auto it=strs.begin(); it!=strs.end(); ++it) {
            matchGeneratedTokens(&tok, it->first, it->second);
        }
    }
    GIVEN("invalid strings") {
        std::vector<std::pair<String, std::vector<std::tuple<JsonTokenizer::Token, JsonTokenizer::ParseError, String>>>> strs = {
            {"\"", {{JsonTokenizer::Token::ERR, JsonTokenizer::ParseError::UNTERMINATED_STR, ""}}},
            {"\"a string", {{JsonTokenizer::Token::ERR, JsonTokenizer::ParseError::UNTERMINATED_STR, "a string"}}},
            {"a string\"", {{JsonTokenizer::Token::ERR, JsonTokenizer::ParseError::UNQUOTED_STR, "a string\""}}},
            {"a string", {{JsonTokenizer::Token::ERR, JsonTokenizer::ParseError::UNQUOTED_STR, "a string"}}},
            {"[\"abc\", def\", \"ghi\"]", {{JsonTokenizer::Token::ARR_START, JsonTokenizer::ParseError::NaE, ""},
                                    {JsonTokenizer::Token::STR, JsonTokenizer::ParseError::NaE, "abc"},
                                    {JsonTokenizer::Token::COMMA, JsonTokenizer::ParseError::NaE, ""},
                                    {JsonTokenizer::Token::ERR, JsonTokenizer::ParseError::UNQUOTED_STR, "def\""},
                                    {JsonTokenizer::Token::COMMA, JsonTokenizer::ParseError::NaE, ""},
                                    {JsonTokenizer::Token::STR, JsonTokenizer::ParseError::NaE, "ghi"},
                                    {JsonTokenizer::Token::ARR_END, JsonTokenizer::ParseError::NaE, ""}}}
        };

        for(auto it=strs.begin(); it!=strs.end(); ++it) {
            matchGeneratedTokensAndErrors(&tok, it->first, it->second);
        }
    }
}

SCENARIO("Tokenize Arrays", "[tokenize]") {
    auto tok = JsonTokenizer();
    GIVEN("empty array") {
        tok.tokenize("[]");
        REQUIRE(tok.hasNext());
        REQUIRE(tok.next(nullptr) == JsonTokenizer::Token::ARR_START);
        REQUIRE(tok.hasNext());
        REQUIRE(tok.next(nullptr) == JsonTokenizer::Token::ARR_END);
        REQUIRE_FALSE(tok.hasNext());
    }

    GIVEN("arrays of numbers") {
        std::vector<std::pair<String, std::vector<std::pair<JsonTokenizer::Token, String>>>> arrs = {
            {"[1,2,3,4]", 
                {{JsonTokenizer::Token::ARR_START, ""}, {JsonTokenizer::Token::NUM, "1"}, {JsonTokenizer::Token::COMMA, ""}, {JsonTokenizer::Token::NUM, "2"},
                {JsonTokenizer::Token::COMMA, ""}, {JsonTokenizer::Token::NUM, "3"}, {JsonTokenizer::Token::COMMA, ""}, {JsonTokenizer::Token::NUM, "4"}, 
                {JsonTokenizer::Token::ARR_END, ""}}},
            {"[-12,-123124.123,0.523,123.2e-123]", 
                {{JsonTokenizer::Token::ARR_START, ""}, {JsonTokenizer::Token::NUM, "-12"}, {JsonTokenizer::Token::COMMA, ""}, 
                {JsonTokenizer::Token::NUM, "-123124.123"}, {JsonTokenizer::Token::COMMA, ""}, {JsonTokenizer::Token::NUM, "0.523"}, {JsonTokenizer::Token::COMMA, ""},
                {JsonTokenizer::Token::NUM, "123.2e-123"}, {JsonTokenizer::Token::ARR_END, ""}}}
        };

        for(auto it=arrs.begin(); it!=arrs.end(); ++it) {
            matchGeneratedTokens(&tok, it->first, it->second);
        }
    }
}

/**
 * Test if 'JsonTokenizer::peek' and 'JsonTokenizer::next' return the same Tokens
 */
SCENARIO("peek() == next()") {
    auto tok = JsonTokenizer();
    tok.tokenize(new StringStream(jsonExample1));
    while(tok.hasNext()) {
        String peekVal = "";
        String nextVal = "";
        auto peek = tok.peek(&peekVal);
        auto next = tok.next(&nextVal);

        REQUIRE(peek == next);
        REQUIRE_FALSE(peek == JsonTokenizer::Token::NaT);
        REQUIRE(peekVal == nextVal);
    }
    REQUIRE_FALSE(tok.hasNext());
}

TEST_CASE("Benchmark tokenizing numbers", "[!hide],[benchmark]") {
    auto tok = JsonTokenizer();
    String json = "[-626817443,-881044913.4317663,-1240190086.148649,-1517826248.6998155,1688970350,1617252167.7838342,-776818085,-1250988003.7511384,1542271282,-1716647248.7116332,-783164903,99430472,999363516.0311377,-432770344,-1561019839,-1862398603.5401747,-776413978.5226648,1467832084,1908788867,-546859749.4767473,-225100767,174427538.01609302,2037788853,2006482759,-9459054,281221418.6285808,-269039325,-1940868180.8921149,-212683153,-709268343.4056752]";

    BENCHMARK("Tokenizer array of numbers") {
        tok.tokenize(json);
        while(tok.hasNext()) {
            String buf = "";
            tok.next(&buf);
        }
    }
}