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
    currentToken = Token::NaT;
    currentVal = "";
    this->is = is;
}

bool JsonTokenizer::hasNext() {
    return is->hasNext();
}

JsonTokenizer::Token JsonTokenizer::peek(String* buf) {
    if(currentToken == Token::NaT) {
        currentToken = next(&currentVal);
    }

    if(buf != nullptr) *buf = currentVal;
    return currentToken;
}

JsonTokenizer::Token JsonTokenizer::next(String* buf) {
    if(currentToken != Token::NaT) {
        Token tmp = currentToken;
        if(buf != nullptr) *buf = currentVal;

        currentToken = Token::NaT;
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
            if(!readStr(buf != nullptr)) return Token::ERR;
            if(buf != nullptr) *buf = currentVal;
            return Token::STR;
        } else if(c == 't') {
            if(!matchStr("true", 4)) return Token::ERR;
            return Token::KW_TRUE;
        } else if(c == 'f') {
            if(!matchStr("false", 5)) return Token::ERR;
            return Token::KW_FALSE;
        } else if(c == 'n') {
            if(!matchStr("null", 4)) return Token::ERR;
            return Token::KW_NULL;
        } else if(Json::isDecDigit(c) || c == '-') {
            if (!readInt(buf != nullptr)) return Token::ERR;
            if(buf != nullptr) *buf = currentVal;
            return Token::INT;
        } else if(c == '.') {
            if(!readFrac(buf != nullptr)) return Token::ERR;
            if(buf != nullptr) *buf = currentVal;
            return Token::FRAC;
        } else if(c == 'e' || c == 'E') {
            if(!readExp(buf != nullptr)) return Token::ERR;
            if(buf != nullptr) *buf = currentVal;
            return Token::EXP;
        } else {
            *buf = new char[1]{is->next()};
            return Token::ERR;
        }
    }

    *buf = "eof";
    return Token::ERR;
}

const char* JsonTokenizer::tokenToStr(Token t) {
    switch(t) {
        case Token::NaT: return "NaT";
        case Token::OBJ_START: return "{";
        case Token::OBJ_END: return "}";
        case Token::ARR_START: return "[";
        case Token::ARR_END: return "]";
        case Token::COLON: return ":";
        case Token::COMMA: return ",";
        case Token::INT: return "INT";
        case Token::FRAC: return "FRAC";
        case Token::EXP: return "EXP";
        case Token::STR: return "STR";
        case Token::KW_NULL: return "null";
        case Token::KW_TRUE: return "true";
        case Token::KW_FALSE: return "false";
        case Token::ERR: return "ERR";
    }
}

/////////////////////
// Private Methods //
/////////////////////

void JsonTokenizer::skipWhitespace() const {
    while(Json::isWhitespace(is->peek())) {
        is->next();
    }
}

bool JsonTokenizer::readInt(bool capture) {
    currentVal = "";
    bool result = false;

    if(is->peek() == '-') {
        if(capture) currentVal += is->next();
        else is->next();
    }

    // An integer starting with 0 cannot be followed by any digits
    bool startsWithZero = false;
    if(is->hasNext() && is->peek() == '0') { 
        if(capture) currentVal += is->next();
        else is->next();
        startsWithZero = true;
        result = true;
    }

    while(is->hasNext() && Json::isDecDigit(is->peek())) {
        if(capture) currentVal += is->next();
        else is->next();
        result = !startsWithZero;
    }
    return result;
}

bool JsonTokenizer::readFrac(bool capture) {
    is->next(); // Skip '.'

    // A fraction has to have at least one digit
    if(Json::isDecDigit(is->peek())) {
        if(capture) currentVal += is->next();
        else is->next();
    } else return false;

    while(is->hasNext() && Json::isDecDigit(is->peek())) {
        if(capture) currentVal += is->next();
        else is->next();       
    }
    return true;
}

bool JsonTokenizer::readExp(bool capture) {
    is->next(); // Skip 'e' or 'E'

    // Sign is optional
    if(is->peek() == '-' || is->peek() == '+') {
        if(capture) currentVal += is->next();
        else is->next();
    }

    // An exponente has to have at least one digit
    if(Json::isDecDigit(is->peek())) {
        if(capture) currentVal += is->next();
        else is->next();
    } else return false;

    while(is->hasNext() && Json::isDecDigit(is->peek())) {
        if(capture) currentVal += is->next();
        else is->next();
    }
    return true;
}

bool JsonTokenizer::readStr(bool capture) {
    currentVal = "";
    if(is->peek() != '"') {
        errorCode = ParseError::MISSING_QUOTE;
        return false;
    }
    is->next();

    while(is->hasNext()) {
        if(is->peek() == '\\') {
            is->next();
            if(is->peek() == 'u') {
                // TODO ...
            } else if(Json::isEscapeable(is->peek())) {
                if(capture) currentVal += Json::escape(is->next());
                else is->next();
            } else {
                errorCode = ParseError::UNESCAPEABLE_CHAR;
                return false;
            }
        } else if(is->peek() == '"') {
            is->next();
            return true;
        } else {
            if(capture) currentVal += is->next();
            else is->next();
        }
    }

    // String wasn't closed properly
    errorCode = ParseError::MISSING_QUOTE;
    return false;
}

bool JsonTokenizer::matchStr(const char kw[], size_t length) {
    for(int i=0; i<length; i++) {
        if(!is->hasNext() || kw[i] != is->peek()) return false;
        else is->next();
    }
    return true;
}