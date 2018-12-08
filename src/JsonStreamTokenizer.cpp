#include "JsonStreamTokenizer.h"
#include <cctype>
#include <iostream>
#include <Streams/StringStream.h>

///////////////////////////////
// Class JsonStreamTokenizer //
///////////////////////////////

JsonStreamTokenizer::JsonStreamTokenizer() {}
JsonStreamTokenizer::~JsonStreamTokenizer() {
    delete is;
}

bool JsonStreamTokenizer::isWhitespace(const char c) const {return c == ' ' || c == '\r' || c == '\n' || c == '\t';}

void JsonStreamTokenizer::tokenize(String str) {tokenize(new StringStream(str));}

void JsonStreamTokenizer::tokenize(InputStream* is) {
    this->is = is;
}

bool JsonStreamTokenizer::hasNext() {
    return is->hasNext();
}

JsonStreamTokenizer::Token JsonStreamTokenizer::peek() {
    if(current.type == Token::Type::TOKEN_NULL) current = next();
    return current;
}

void JsonStreamTokenizer::skipWhitespace() const {
    while(isWhitespace(is->peek())) {
        is->next();
    }
}

String readString(InputStream* is) {
    String str = "";
    while(is->hasNext()) {
        if(is->peek() == '"') break;
        else str += is->next();
    }
    return str;
}

bool readBool(InputStream* is) {
    if(is->peek() == 't') {
        const char buff[] = {'t', 'r', 'u', 'e'};
        for(int i=0; i<sizeof(buff)/sizeof(char); i++) {
            if(!is->hasNext() || buff[i] != is->peek()) {
                //ERRORF("Unexpected char in Bool: '%c'", is->peek()) return false;
                return false; 
            }
            else is->next();
        }
    } else {
        const char buff[] = {'f', 'a', 'l', 's', 'e'};
        for(int i=0; i<sizeof(buff)/sizeof(char); i++) {
            if(!is->hasNext() || buff[i] != is->peek()) {
                //ERRORF("Unexpected char in Bool: '%c'", is->peek()) return false;
                return false;
            }
            else is->next();
        }
    }
    return true;
}

bool readNull(InputStream* is) {
    const char buff[] = {'n', 'u', 'l', 'l'};
    for(int i=0; i<sizeof(buff)/sizeof(char); i++) {
        if(!is->hasNext() || buff[i] != is->peek()) {
            //ERRORF("Unexpected char in null: '%c'", is->peek()) return false;
            return false; 
        }
        else is->next();
    }
    return true;
}

String readNumber(InputStream* is) {
    String num = "";
    while(is->hasNext()) {
        char c = is->peek();
        if(isdigit(c) || c == '.' || c == '-' || c == '+' || c == 'e' || c == 'E') num += is->next();
        else break;
    }
    return num;
}

JsonStreamTokenizer::Token JsonStreamTokenizer::next() {
    if(current.type != Token::Type::TOKEN_NULL) {
        Token tmp = current;
        current = Token{Token::Type::TOKEN_NULL, ""};
        return tmp;
    }

    while(is->hasNext()) {
        skipWhitespace();
        const char c = is->peek();
        if(c == '[') return Token{Token::Type::ARR_START, String(is->next())};
        else if(c == ']') return Token{Token::Type::ARR_END, String(is->next())};
        else if(c == '{') return Token{Token::Type::OBJ_START, String(is->next())};
        else if(c == '}') return Token{Token::Type::OBJ_END, String(is->next())};
        else if(c == ':') return Token{Token::Type::COLON, String(is->next())};
        else if(c == ',') return Token{Token::Type::COMMA, String(is->next())};
        else if(c == '"') { 
            is->next();
            String str = readString(is);
            if(!is->hasNext() || is->peek() != '"') return Token{Token::Type::TOKEN_ERROR, str};
            is->next();
            return Token{Token::Type::STR, str};
        } else if(c == 't' || c == 'f'){
            if(!readBool(is)) return Token{Token::Type::TOKEN_ERROR, String(c)};
            return Token{Token::Type::BOOL, c == 't' ? "true" : "false"};
        } else if(c == 'n') {
            if(!readNull(is)) return Token{Token::Type::TOKEN_ERROR, String(c)};
            return Token{Token::Type::NUL, "null"};
        } else if(isdigit(c) || c == '-') {
            String num = readNumber(is);
            return Token{Token::Type::NUM, num};
        } return Token{Token::Type::TOKEN_ERROR, String(is->next())};
    }

    return Token{Token::Type::TOKEN_ERROR, "End of Stream"};
}

/////////////////
// Class Token //
/////////////////

const char* JsonStreamTokenizer::Token::typeToStr(Type t) {
    switch(t) {
        case Type::DOC_START: return "DOC_START";
        case Type::DOC_END: return "DOC_END";
        case Type::OBJ_START: return "{";
        case Type::OBJ_END: return "}";
        case Type::ARR_START: return "[";
        case Type::ARR_END: return "]";
        case Type::COLON: return ":";
        case Type::COMMA: return ",";
        case Type::KEY: return "KEY"; 
        case Type::NUM: return "NUM";
        case Type::STR: return "STR";
        case Type::NUL: return "NUL";
        case Type::BOOL: return "BOOL";
        case Type::TOKEN_ERROR: return "ERROR";
        case Type::TOKEN_NULL: return "TOKEN_NULL";
    }
}