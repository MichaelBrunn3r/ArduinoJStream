#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <iostream>
#include <utility>

#include <Arduino.h>
#include <JsonTokenizer.h>
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
    REQUIRE(String(JsonTokenizer::tokenToStr(JsonTokenizer::Token::NaT)) == "NaT");
    REQUIRE(String(JsonTokenizer::tokenToStr(JsonTokenizer::Token::OBJ_START)) == "{");
    REQUIRE(String(JsonTokenizer::tokenToStr(JsonTokenizer::Token::OBJ_END)) == "}");
    REQUIRE(String(JsonTokenizer::tokenToStr(JsonTokenizer::Token::ARR_START)) == "[");
    REQUIRE(String(JsonTokenizer::tokenToStr(JsonTokenizer::Token::ARR_END)) == "]");
    REQUIRE(String(JsonTokenizer::tokenToStr(JsonTokenizer::Token::COLON)) == ":");
    REQUIRE(String(JsonTokenizer::tokenToStr(JsonTokenizer::Token::COMMA)) == ",");
    REQUIRE(String(JsonTokenizer::tokenToStr(JsonTokenizer::Token::INT)) == "INT");
    REQUIRE(String(JsonTokenizer::tokenToStr(JsonTokenizer::Token::FRAC)) == "FRAC");
    REQUIRE(String(JsonTokenizer::tokenToStr(JsonTokenizer::Token::EXP)) == "EXP");
    REQUIRE(String(JsonTokenizer::tokenToStr(JsonTokenizer::Token::STR)) == "STR");
    REQUIRE(String(JsonTokenizer::tokenToStr(JsonTokenizer::Token::KW_NULL)) == "null");
    REQUIRE(String(JsonTokenizer::tokenToStr(JsonTokenizer::Token::KW_TRUE)) == "true");
    REQUIRE(String(JsonTokenizer::tokenToStr(JsonTokenizer::Token::KW_FALSE)) == "false");
    REQUIRE(String(JsonTokenizer::tokenToStr(JsonTokenizer::Token::ERR)) == "ERR");
}

/**
 * Test if the Tokenizer tokenizes the basic building block Tokens (everything except a String or a Number) correctly
 */
SCENARIO("Tokenize basic Tokens") {
    auto tok = JsonTokenizer();
    std::pair<String, JsonTokenizer::Token> tokens[] = 
        {{"{", JsonTokenizer::Token::OBJ_START},{"}", JsonTokenizer::Token::OBJ_END},{"[", JsonTokenizer::Token::ARR_START},{"]", JsonTokenizer::Token::ARR_END}, 
        {":", JsonTokenizer::Token::COLON}, {",", JsonTokenizer::Token::COMMA}, {"null", JsonTokenizer::Token::KW_NULL}, {"true", JsonTokenizer::Token::KW_TRUE}, 
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