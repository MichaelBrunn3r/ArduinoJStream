#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <iostream>
#include <utility>
#include <vector>

#include <Arduino.h>
#include <JsonTokenizer.h>
#include <Streams/StringStream.h>

std::ostream& operator << ( std::ostream& os, JsonTokenizer::Token const& value ) {
    os << JsonTokenizer::tokenToStr(value);
    return os;
}

void matchGeneratedTokens(JsonTokenizer* tok, String json, std::vector<JsonTokenizer::Token> tokens) {
    tok->tokenize(json);
    for(int k=0; k<tokens.size(); k++) {
        REQUIRE(tok->next(nullptr) == tokens[k]);
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

SCENARIO("Token Type to String") {
    REQUIRE(String(JsonTokenizer::tokenToStr(JsonTokenizer::Token::NaT)) == "NaT");
    REQUIRE(String(JsonTokenizer::tokenToStr(JsonTokenizer::Token::OBJ_START)) == "{");
    REQUIRE(String(JsonTokenizer::tokenToStr(JsonTokenizer::Token::OBJ_END)) == "}");
    REQUIRE(String(JsonTokenizer::tokenToStr(JsonTokenizer::Token::ARR_START)) == "[");
    REQUIRE(String(JsonTokenizer::tokenToStr(JsonTokenizer::Token::ARR_END)) == "]");
    REQUIRE(String(JsonTokenizer::tokenToStr(JsonTokenizer::Token::COMMA)) == ",");
    REQUIRE(String(JsonTokenizer::tokenToStr(JsonTokenizer::Token::INT)) == "INT");
    REQUIRE(String(JsonTokenizer::tokenToStr(JsonTokenizer::Token::FRAC)) == "FRAC");
    REQUIRE(String(JsonTokenizer::tokenToStr(JsonTokenizer::Token::EXP)) == "EXP");
    REQUIRE(String(JsonTokenizer::tokenToStr(JsonTokenizer::Token::STR)) == "STR");
    REQUIRE(String(JsonTokenizer::tokenToStr(JsonTokenizer::Token::FIELD_NAME)) == "FIELD_NAME");
    REQUIRE(String(JsonTokenizer::tokenToStr(JsonTokenizer::Token::KW_NULL)) == "null");
    REQUIRE(String(JsonTokenizer::tokenToStr(JsonTokenizer::Token::KW_TRUE)) == "true");
    REQUIRE(String(JsonTokenizer::tokenToStr(JsonTokenizer::Token::KW_FALSE)) == "false");
    REQUIRE(String(JsonTokenizer::tokenToStr(JsonTokenizer::Token::ERR)) == "ERR");
}

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

SCENARIO("Tokenize numbers") {
    auto tok = JsonTokenizer();
    GIVEN("valid numbers") {
        std::vector<std::pair<String, std::vector<JsonTokenizer::Token>>> nums = {
            {"0", {JsonTokenizer::Token::INT}},
            {"-0", {JsonTokenizer::Token::INT}},
            {"-97812467", {JsonTokenizer::Token::INT}},
            {"123151", {JsonTokenizer::Token::INT}},
            {".3123", {JsonTokenizer::Token::FRAC}},
            {"0.0", {JsonTokenizer::Token::INT, JsonTokenizer::Token::FRAC}},
            {"-1.321", {JsonTokenizer::Token::INT, JsonTokenizer::Token::FRAC}},
            {"e4123", {JsonTokenizer::Token::EXP}},
            {"E+352532", {JsonTokenizer::Token::EXP}},
            {"E-6775456", {JsonTokenizer::Token::EXP}},
            {"12e4234", {JsonTokenizer::Token::INT, JsonTokenizer::Token::EXP}},
            {".642E+345", {JsonTokenizer::Token::FRAC, JsonTokenizer::Token::EXP}},
            {"-0.123e-234", {JsonTokenizer::Token::INT, JsonTokenizer::Token::FRAC, JsonTokenizer::Token::EXP}}
        };
            
        for(int i=0; i<nums.size(); i++) {
            matchGeneratedTokens(&tok, nums[i].first, nums[i].second);
        }
    }
    GIVEN("invalid numbers") {
        std::vector<std::pair<String, std::vector<JsonTokenizer::Token>>> nums = {
            {"-", {JsonTokenizer::Token::ERR}},
            {"00", {JsonTokenizer::Token::ERR}},
            {"014112", {JsonTokenizer::Token::ERR}},
            {"-.12", {JsonTokenizer::Token::ERR, JsonTokenizer::Token::FRAC}},
            {"-E-3423", {JsonTokenizer::Token::ERR, JsonTokenizer::Token::EXP}}
        };
            
        for(int i=0; i<nums.size(); i++) {
            matchGeneratedTokens(&tok, nums[i].first, nums[i].second);
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