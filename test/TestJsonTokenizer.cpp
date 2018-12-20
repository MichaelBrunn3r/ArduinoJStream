#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <iostream>
#include <utility>
#include <vector>
#include <tuple>

#include <Arduino.h>
#include <JsonTokenizer.h>
#include <Streams/StringStream.h>

std::ostream& operator << ( std::ostream& os, JsonTokenizer::Token const& value ) {
    os << JsonTokenizer::tokenToStr(value);
    return os;
}

std::ostream& operator << ( std::ostream& os, JsonTokenizer::ParseError const& value ) {
    os << JsonTokenizer::errorToStr(value);
    return os;
}

void matchGeneratedTokens(JsonTokenizer* tok, String json, std::vector<std::pair<JsonTokenizer::Token, String>> tokens) {
    tok->tokenize(json);
    for(int k=0; k<tokens.size(); k++) {
        String buf = "";
        REQUIRE(tok->next(&buf) == tokens[k].first);
        REQUIRE(buf == tokens[k].second);
    }
    REQUIRE_FALSE(tok->hasNext());
}

void matchGeneratedTokensAndErrors(JsonTokenizer* tok, String json, std::vector<std::tuple<JsonTokenizer::Token, JsonTokenizer::ParseError, String>> tokens) {
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
    std::pair<String, JsonTokenizer::Token> tokens[] = 
        {{"{", JsonTokenizer::Token::OBJ_START},{"}", JsonTokenizer::Token::OBJ_END},{"[", JsonTokenizer::Token::ARR_START},{"]", JsonTokenizer::Token::ARR_END}, 
        {",", JsonTokenizer::Token::COMMA}, {"null", JsonTokenizer::Token::KW_NULL}, {"true", JsonTokenizer::Token::KW_TRUE}, 
        {"false", JsonTokenizer::Token::KW_FALSE}};
    
    for(int i=0; i<sizeof(tokens)/sizeof(std::pair<String, JsonTokenizer::Token>); i++) {
        tok.tokenize(new StringStream(tokens[i].first));
        REQUIRE(tok.hasNext());
        String buf = "";
        REQUIRE(tok.next(&buf) == tokens[i].second);
        REQUIRE(buf == "");
        REQUIRE_FALSE(tok.hasNext());
    }
}

SCENARIO("Tokenize numbers", "[tokenize]") {
    auto tok = JsonTokenizer();

    // INTEGERS
    GIVEN("valid integers") {
        std::vector<std::pair<String, std::vector<std::pair<JsonTokenizer::Token, String>>>> ints = {
            {"0", {{JsonTokenizer::Token::INT, "0"}}},
            {"-0", {{JsonTokenizer::Token::INT, "-0"}}},
            {"2834719075129051986198309182370", {{JsonTokenizer::Token::INT, "2834719075129051986198309182370"}}},
            {"-4192359128129591469102380120350", {{JsonTokenizer::Token::INT, "-4192359128129591469102380120350"}}}
        };

        for(int i=0; i<ints.size(); i++) {
            matchGeneratedTokens(&tok, ints[i].first, ints[i].second);
        }
    }
    GIVEN("invalid integers") {
        std::vector<std::pair<String, std::vector<std::tuple<JsonTokenizer::Token, JsonTokenizer::ParseError, String>>>> ints = {
            {"-", {{JsonTokenizer::Token::ERR, JsonTokenizer::ParseError::UNEXPECTED_EOS, "-"}}},
            {"0234", {{JsonTokenizer::Token::ERR, JsonTokenizer::ParseError::MALFORMED_INT, "0234"}}},
            {"-0234", {{JsonTokenizer::Token::ERR, JsonTokenizer::ParseError::MALFORMED_INT, "-0234"}}}
        };

        for(int i=0; i<ints.size(); i++) {
            matchGeneratedTokensAndErrors(&tok, ints[i].first, ints[i].second);
        }
    }

    // FRACTIONS
    GIVEN("valid fractions") {
        std::vector<std::pair<String, std::vector<std::pair<JsonTokenizer::Token, String>>>> ints = {
            {".0", {{JsonTokenizer::Token::FRAC, "0"}}},
            {".6343", {{JsonTokenizer::Token::FRAC, "6343"}}},
            {".0006343000", {{JsonTokenizer::Token::FRAC, "0006343000"}}}
        };

        for(int i=0; i<ints.size(); i++) {
            matchGeneratedTokens(&tok, ints[i].first, ints[i].second);
        }
    }
    GIVEN("invalid fractions") {
        std::vector<std::pair<String, std::vector<std::tuple<JsonTokenizer::Token, JsonTokenizer::ParseError, String>>>> fracs = {
            {".", {{JsonTokenizer::Token::ERR, JsonTokenizer::ParseError::UNEXPECTED_EOS, ""}}},
            {".a", {{JsonTokenizer::Token::ERR, JsonTokenizer::ParseError::MALFORMED_FRAC, ""},
                    {JsonTokenizer::Token::ERR, JsonTokenizer::ParseError::UNEXPECTED_CHAR, "a"}}},
            {".012.032", {{JsonTokenizer::Token::FRAC, JsonTokenizer::ParseError::NaE, "012"}, 
                            {JsonTokenizer::Token::FRAC, JsonTokenizer::ParseError::NaE, "032"}}}
        };

        for(int i=0; i<fracs.size(); i++) {
            matchGeneratedTokensAndErrors(&tok, fracs[i].first, fracs[i].second);
        }
    }

    // EXPONENTS
    GIVEN("valid exponentes") {
        std::vector<std::pair<String, std::vector<std::pair<JsonTokenizer::Token, String>>>> exps = {
            {"e10", {{JsonTokenizer::Token::EXP, "10"}}},
            {"e0", {{JsonTokenizer::Token::EXP, "0"}}},
            {"e00001", {{JsonTokenizer::Token::EXP, "00001"}}},
            {"E+311", {{JsonTokenizer::Token::EXP, "311"}}}
        };

        for(int i=0; i<exps.size(); i++) {
            matchGeneratedTokens(&tok, exps[i].first, exps[i].second);
        }
    }
    GIVEN("invalid exponents") {
        std::vector<std::pair<String, std::vector<std::tuple<JsonTokenizer::Token, JsonTokenizer::ParseError, String>>>> exps = {
            {"e", {{JsonTokenizer::Token::ERR, JsonTokenizer::ParseError::UNEXPECTED_EOS, ""}}},
            {"E", {{JsonTokenizer::Token::ERR, JsonTokenizer::ParseError::UNEXPECTED_EOS, ""}}},
            {"e+", {{JsonTokenizer::Token::ERR, JsonTokenizer::ParseError::UNEXPECTED_EOS, ""}}},
            {"e-", {{JsonTokenizer::Token::ERR, JsonTokenizer::ParseError::UNEXPECTED_EOS, "-"}}},
            {"e--", {{JsonTokenizer::Token::ERR, JsonTokenizer::ParseError::MALFORMED_EXP, "-"}, 
                        {JsonTokenizer::Token::ERR, JsonTokenizer::ParseError::UNEXPECTED_EOS, "-"}}},
            {"e++", {{JsonTokenizer::Token::ERR, JsonTokenizer::ParseError::MALFORMED_EXP, ""}, 
                        {JsonTokenizer::Token::ERR, JsonTokenizer::ParseError::UNEXPECTED_CHAR, "+"}}}
        };

        for(int i=0; i<exps.size(); i++) {
            matchGeneratedTokensAndErrors(&tok, exps[i].first, exps[i].second);
        }
    }

    // NUMBERS
    GIVEN("valid numbers") {
        std::vector<std::pair<String, std::vector<std::pair<JsonTokenizer::Token, String>>>> nums = {
            {"0.1", {{JsonTokenizer::Token::INT, "0"}, {JsonTokenizer::Token::FRAC, "1"}}},
            {"-0.1", {{JsonTokenizer::Token::INT, "-0"}, {JsonTokenizer::Token::FRAC, "1"}}},
            {"-0.1", {{JsonTokenizer::Token::INT, "-0"}, {JsonTokenizer::Token::FRAC, "1"}}},
            {"0e10", {{JsonTokenizer::Token::INT, "0"}, {JsonTokenizer::Token::EXP, "10"}}},
            {"-0.1E+32", {{JsonTokenizer::Token::INT, "-0"}, {JsonTokenizer::Token::FRAC, "1"}, {JsonTokenizer::Token::EXP, "32"}}}
        };

        for(int i=0; i<nums.size(); i++) {
            matchGeneratedTokens(&tok, nums[i].first, nums[i].second);
        }
    }
    GIVEN("invalid numbers") {
        std::vector<std::pair<String, std::vector<std::tuple<JsonTokenizer::Token, JsonTokenizer::ParseError, String>>>> nums = {
            {"0.1.10", {{JsonTokenizer::Token::INT, JsonTokenizer::ParseError::NaE, "0"},
                        {JsonTokenizer::Token::FRAC, JsonTokenizer::ParseError::NaE, "1"},
                        {JsonTokenizer::Token::FRAC, JsonTokenizer::ParseError::NaE, "10"}}},
            {"0e-12e+2", {{JsonTokenizer::Token::INT, JsonTokenizer::ParseError::NaE, "0"},
                            {JsonTokenizer::Token::EXP, JsonTokenizer::ParseError::NaE, "-12"},
                            {JsonTokenizer::Token::EXP, JsonTokenizer::ParseError::NaE, "2"}}}
        };

        for(int i=0; i<nums.size(); i++) {
            matchGeneratedTokensAndErrors(&tok, nums[i].first, nums[i].second);
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