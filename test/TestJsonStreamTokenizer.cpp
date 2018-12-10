#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <iostream>
#include <utility>

#include <Arduino.h>
#include <JsonStreamTokenizer.h>
#include <Streams/StringStream.h>

static String jsonExample1 = 
    "{ \
        \"string2\": \"Nightman\", \
        \"multiline\": \"I \nam a\nmultiline String\", \
        \"link\": \"https://de.wikipedia.org/wiki/Arduino_(Plattform)\", \
        \"email\": \"ardu@ino.com\", \
        \"int\": 198, \
        \"double\": -2.346326, \
        \"large_num\": 0.124E-123, \
        \"false\": false, \
        \"true\": true, \
        \"null\": null \
    }";

SCENARIO("Token Type to String") {
    REQUIRE(String(JsonStreamTokenizer::tokenToStr(JsonStreamTokenizer::Token::NONE)) == "NONE");
    REQUIRE(String(JsonStreamTokenizer::tokenToStr(JsonStreamTokenizer::Token::OBJ_START)) == "{");
    REQUIRE(String(JsonStreamTokenizer::tokenToStr(JsonStreamTokenizer::Token::OBJ_END)) == "}");
    REQUIRE(String(JsonStreamTokenizer::tokenToStr(JsonStreamTokenizer::Token::ARR_START)) == "[");
    REQUIRE(String(JsonStreamTokenizer::tokenToStr(JsonStreamTokenizer::Token::ARR_END)) == "]");
    REQUIRE(String(JsonStreamTokenizer::tokenToStr(JsonStreamTokenizer::Token::COLON)) == ":");
    REQUIRE(String(JsonStreamTokenizer::tokenToStr(JsonStreamTokenizer::Token::COMMA)) == ",");
    REQUIRE(String(JsonStreamTokenizer::tokenToStr(JsonStreamTokenizer::Token::NUM)) == "NUM");
    REQUIRE(String(JsonStreamTokenizer::tokenToStr(JsonStreamTokenizer::Token::STR)) == "STR");
    REQUIRE(String(JsonStreamTokenizer::tokenToStr(JsonStreamTokenizer::Token::KW_NULL)) == "null");
    REQUIRE(String(JsonStreamTokenizer::tokenToStr(JsonStreamTokenizer::Token::KW_TRUE)) == "true");
    REQUIRE(String(JsonStreamTokenizer::tokenToStr(JsonStreamTokenizer::Token::KW_FALSE)) == "false");
    REQUIRE(String(JsonStreamTokenizer::tokenToStr(JsonStreamTokenizer::Token::ERROR)) == "ERROR");
}

/**
 * Test if the Tokenizer tokenizes the basic building block Tokens (everything except a String or a Number) correctly
 */
SCENARIO("Tokenize basic Tokens") {
    auto tok = JsonStreamTokenizer();
    std::pair<String, JsonStreamTokenizer::Token> tokens[] = 
        {{"{", JsonStreamTokenizer::Token::OBJ_START},{"}", JsonStreamTokenizer::Token::OBJ_END},{"[", JsonStreamTokenizer::Token::ARR_START},{"]", JsonStreamTokenizer::Token::ARR_END}, 
        {":", JsonStreamTokenizer::Token::COLON}, {",", JsonStreamTokenizer::Token::COMMA}, {"null", JsonStreamTokenizer::Token::KW_NULL}, {"true", JsonStreamTokenizer::Token::KW_TRUE}, 
        {"false", JsonStreamTokenizer::Token::KW_FALSE}};
    
    for(int i=0; i<sizeof(tokens)/sizeof(std::pair<String, JsonStreamTokenizer::Token>); i++) {
        tok.tokenize(new StringStream(tokens[i].first));
        REQUIRE(tok.hasNext());
        String buf = "";
        REQUIRE(tok.next(&buf) == tokens[i].second);
        REQUIRE(buf == "");
        REQUIRE_FALSE(tok.hasNext());
    }
}

/**
 * Test if 'JsonStreamTokenizer::peek' and 'JsonStreamTokenizer::next' return the same Tokens
 */
SCENARIO("peek() == next()") {
    auto tok = JsonStreamTokenizer();
    tok.tokenize(new StringStream(jsonExample1));
    while(tok.hasNext()) {
        String peekVal = "";
        String nextVal = "";
        auto peek = tok.peek(&peekVal);
        auto next = tok.next(&nextVal);

        REQUIRE(peek == next);
        REQUIRE_FALSE(peek == JsonStreamTokenizer::Token::NONE);
        REQUIRE(peekVal == nextVal);
    }
    REQUIRE_FALSE(tok.hasNext());
}