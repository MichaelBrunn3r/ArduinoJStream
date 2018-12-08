#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <iostream>

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
        \"large_num\": 0.124E-123 \
    }";

SCENARIO("Token Type to String") {
    REQUIRE(String(JsonStreamTokenizer::Token::typeToStr(JsonStreamTokenizer::Token::Type::DOC_START)) == "DOC_START");
    REQUIRE(String(JsonStreamTokenizer::Token::typeToStr(JsonStreamTokenizer::Token::Type::DOC_END)) == "DOC_END");
    REQUIRE(String(JsonStreamTokenizer::Token::typeToStr(JsonStreamTokenizer::Token::Type::OBJ_START)) == "{");
    REQUIRE(String(JsonStreamTokenizer::Token::typeToStr(JsonStreamTokenizer::Token::Type::OBJ_END)) == "}");
    REQUIRE(String(JsonStreamTokenizer::Token::typeToStr(JsonStreamTokenizer::Token::Type::ARR_START)) == "[");
    REQUIRE(String(JsonStreamTokenizer::Token::typeToStr(JsonStreamTokenizer::Token::Type::ARR_END)) == "]");
    REQUIRE(String(JsonStreamTokenizer::Token::typeToStr(JsonStreamTokenizer::Token::Type::COLON)) == ":");
    REQUIRE(String(JsonStreamTokenizer::Token::typeToStr(JsonStreamTokenizer::Token::Type::COMMA)) == ",");
    REQUIRE(String(JsonStreamTokenizer::Token::typeToStr(JsonStreamTokenizer::Token::Type::KEY)) == "KEY");
    REQUIRE(String(JsonStreamTokenizer::Token::typeToStr(JsonStreamTokenizer::Token::Type::NUM)) == "NUM");
    REQUIRE(String(JsonStreamTokenizer::Token::typeToStr(JsonStreamTokenizer::Token::Type::STR)) == "STR");
    REQUIRE(String(JsonStreamTokenizer::Token::typeToStr(JsonStreamTokenizer::Token::Type::NUL)) == "NUL");
    REQUIRE(String(JsonStreamTokenizer::Token::typeToStr(JsonStreamTokenizer::Token::Type::BOOL)) == "BOOL");
    REQUIRE(String(JsonStreamTokenizer::Token::typeToStr(JsonStreamTokenizer::Token::Type::TOKEN_ERROR)) == "ERROR");
    REQUIRE(String(JsonStreamTokenizer::Token::typeToStr(JsonStreamTokenizer::Token::Type::TOKEN_NULL)) == "TOKEN_NULL");
}

SCENARIO("peek() and next() return the same Token") {
    String json = jsonExample1;
    auto tok = JsonStreamTokenizer();
    tok.tokenize(new StringStream(json));
    while(tok.hasNext()) {
        auto peek = tok.peek();
        auto next = tok.next();
        REQUIRE_FALSE(peek.type == JsonStreamTokenizer::Token::Type::TOKEN_ERROR);
        REQUIRE(peek.type == next.type);
        REQUIRE(peek.val == next.val);
    }
    REQUIRE_FALSE(tok.hasNext());
}