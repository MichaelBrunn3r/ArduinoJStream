#include "JsonTokenizer.h"
#include <iostream>
#include <Streams/StringStream.h>
#include <JsonUtils.h>

JsonTokenizer::JsonTokenizer() {}
JsonTokenizer::~JsonTokenizer() {
    delete is;
}

void JsonTokenizer::tokenize(String str) {tokenize(new StringStream(str));}

void JsonTokenizer::tokenize(InputStream* is) {
    currentToken = Token::NONE;
    currentVal = "";
    this->is = is;
}

bool JsonTokenizer::hasNext() {
    return is->hasNext();
}

JsonTokenizer::Token JsonTokenizer::peek(String* buf) {
    if(currentToken == Token::NONE) {
        currentToken = next(&currentVal);
    }

    *buf = currentVal;
    return currentToken;
}

JsonTokenizer::Token JsonTokenizer::next(String* buf) {
    if(currentToken != Token::NONE) {
        Token tmp = currentToken;
        *buf = currentVal;

        currentToken = Token::NONE;
        currentVal = "";

        return tmp;
    }

    while(is->hasNext()) {
        skipWhitespace();
        const char c = is->peek();
        if(c == '[') {is->next(); return Token::ARR_START;}
        else if(c == ']') {is->next(); return Token::ARR_END;}
        else if(c == '{') {is->next(); return Token::OBJ_START;}
        else if(c == '}') {is->next(); return Token::OBJ_END;}
        else if(c == ':') {is->next(); return Token::COLON;}
        else if(c == ',') {is->next(); return Token::COMMA;}
        else if(c == '"') { 
            is->next(); // Skip opening "
            readStr();
            *buf = currentVal;
            if(!is->hasNext() || is->peek() != '"') return Token::ERROR;
            is->next(); // Skip closing "
            return Token::STR;
        } else if(c == 't' || c == 'f' || c == 'n'){
            Token t = Token::ERROR;

            if(c == 't' && matchStr("true")) t = Token::KW_TRUE;
            else if(c == 'f' && matchStr("false")) t = Token::KW_FALSE;
            else if(c == 'n' && matchStr("null")) t = Token::KW_NULL;

            if(t == Token::ERROR) *buf = new char[1]{c};
            return t;
        } else if(isNumStart(c)) {
            readNum();
            *buf = currentVal;
            return Token::NUM;
        } else {
            *buf = new char[1]{is->next()};
            return Token::ERROR;
        }
    }

    *buf = "eof";
    return Token::ERROR;
}

const char* JsonTokenizer::tokenToStr(Token t) {
    switch(t) {
        case Token::NONE: return "NONE";
        case Token::OBJ_START: return "{";
        case Token::OBJ_END: return "}";
        case Token::ARR_START: return "[";
        case Token::ARR_END: return "]";
        case Token::COLON: return ":";
        case Token::COMMA: return ",";
        case Token::NUM: return "NUM";
        case Token::STR: return "STR";
        case Token::KW_NULL: return "null";
        case Token::KW_TRUE: return "true";
        case Token::KW_FALSE: return "false";
        case Token::ERROR: return "ERROR";
    }
}

/////////////////////
// Private Methods //
/////////////////////

void JsonTokenizer::skipWhitespace() const {
    while(isWhitespace(is->peek())) {
        is->next();
    }
}

void JsonTokenizer::readNum() {
    currentVal = "";
    while(is->hasNext()) {
        char c = is->peek();
        if(isNumDigit(c)) currentVal += is->next();
        else break;
    }
}

void JsonTokenizer::readStr() {
    currentVal = "";
    while(is->hasNext()) {
        if(is->peek() == '"') break;
        else currentVal += is->next();
    }
}

bool JsonTokenizer::matchStr(const char kw[]) {
    for(int i=0; kw[i] != '\0'; i++) {
        if(!is->hasNext() || kw[i] != is->peek()) return false;
        else is->next();
    }
    return true;
}