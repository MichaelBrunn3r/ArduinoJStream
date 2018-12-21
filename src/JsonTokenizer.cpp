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
        currentVal = "";
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

        // Check if the character is the start of a number
        if(Json::isDecDigit(is->peek()) || is->peek() == '-') {
            // Integers values are handled in special way, because they do not have a usefull prefix (like '"', '.' or 'e').
            // Calling 'is->next()' would unneccessarily complicate reading an Integer.

            if (!readInt(buf)) return Token::ERR;
            else return Token::INT;
        }

        const char c = is->next();
        if(c == '[') return Token::ARR_START;
        else if(c == ']') return Token::ARR_END;
        else if(c == '{') return Token::OBJ_START;
        else if(c == '}') return Token::OBJ_END;
        else if(c == ',') return Token::COMMA;
        else if(c == '"') { 
            // Read String
            if(!readStr(buf)) return Token::ERR;

            // Test if the String is a field name
            skipWhitespace();
            if(is->hasNext() && is->peek() == ':') {
                is->next();
                return Token::FIELD_NAME;
            } else return Token::STR;
        } else if(c == 't') {
            if(!matchStr("rue", 3)) return Token::ERR;
            return Token::KW_TRUE;
        } else if(c == 'f') {
            if(!matchStr("alse", 4)) return Token::ERR;
            return Token::KW_FALSE;
        } else if(c == 'n') {
            if(!matchStr("ull", 3)) return Token::ERR;
            return Token::KW_NULL;
        } else if(c == '.') {
            if(!readFrac(buf)) return Token::ERR;
            else return Token::FRAC;
        } else if(c == 'e' || c == 'E') {
            if(!readExp(buf)) return Token::ERR;
            else return Token::EXP;
        } else {
            // If the char is unexpexted, assume its an unquoted string
            errorCode = ParseError::UNQUOTED_STR;
            if(buf != nullptr) *buf += c;

            // Read the rest of the unquoted string
            while(is->hasNext()) {
                char c = is->peek();
                if(c == ',' || c == ']' || c == '}') break; // chars that can terminate a json value

                if(buf!= nullptr) *buf += c;
                is->next();
            }
            return Token::ERR;
        }
    }

    errorCode = ParseError::UNEXPECTED_EOS;
    return Token::ERR;
}

const char* JsonTokenizer::tokenToStr(Token t) {
    switch(t) {
        case Token::NaT: return "NaT";
        case Token::OBJ_START: return "{";
        case Token::OBJ_END: return "}";
        case Token::ARR_START: return "[";
        case Token::ARR_END: return "]";
        case Token::COMMA: return ",";
        case Token::INT: return "INT";
        case Token::FRAC: return "FRAC";
        case Token::EXP: return "EXP";
        case Token::STR: return "STR";
        case Token::FIELD_NAME: return "FIELD_NAME";
        case Token::KW_NULL: return "null";
        case Token::KW_TRUE: return "true";
        case Token::KW_FALSE: return "false";
        case Token::ERR: return "ERR";
    }
}

const char* JsonTokenizer::errorToStr(ParseError e) {
    switch(e) {
        case ParseError::NaE: return "NaE";
        case ParseError::UNEXPECTED_EOS: return "UNEXPECTED_EOS"; 
        case ParseError::UNQUOTED_STR : return "UNQUOTED_STR";
        case ParseError::MALFORMED_INT: return "MALFORMED_INT";
        case ParseError::MALFORMED_FRAC: return "MALFORMED_FRAC";
        case ParseError::MALFORMED_EXP: return "MALFORMED_EXP";
        case ParseError::UNESCAPEABLE_CHAR: return "UNESCAPEABLE_CHAR";
        case ParseError::UNTERMINATED_STR: return "UNTERMINATED_STR";
    }
}

/////////////////////
// Private Methods //
/////////////////////

void JsonTokenizer::skipWhitespace() const {
    while(is->hasNext() && Json::isWhitespace(is->peek())) {
        is->next();
    }
}

bool JsonTokenizer::readInt(String* buf) {
    // Optional '-' prefix
    if(is->peek() == '-') {
        if(buf != nullptr) *buf += is->next();
        else is->next();
    }

    if(!hasNext()) {errorCode = ParseError::UNEXPECTED_EOS; return false;}

    char firstDigit = 0; // First decimal digit
    bool moreThanOneDigit = false; // Indicates if the Integer has more than one decimal digit

    // Reads digits even if the Integer is malformed
    while(is->hasNext() && Json::isDecDigit(is->peek())) {
        char digit = is->next();
        if(buf != nullptr) *buf += digit;

        if(firstDigit == 0) firstDigit = digit;
        else moreThanOneDigit = true;
    }

    // An Integer has to have at least one digit and a starting 0 cannot be followed by a digit.
    if(firstDigit == 0 || firstDigit == '0' && moreThanOneDigit) {
        errorCode = ParseError::MALFORMED_INT; 
        return false;
    } else return true;
}

bool JsonTokenizer::readFrac(String* buf) {
    if(!is->hasNext()) {errorCode = ParseError::UNEXPECTED_EOS; return false;}
    
    // A fraction has to have at least one digit
    bool atLeastOneDigit = Json::isDecDigit(is->peek());

    while(is->hasNext() && Json::isDecDigit(is->peek())) {
        if(buf != nullptr) *buf += is->next();
        else is->next();       
    }

    if(!atLeastOneDigit) errorCode = ParseError::MALFORMED_FRAC;
    return atLeastOneDigit;
}

bool JsonTokenizer::readExp(String* buf) {
    // Sign is optional
    if(is->hasNext() && is->peek() == '-') {
        if(buf != nullptr) *buf += is->next();
        else is->next();
    } else if(is->hasNext() && is->peek() == '+') is->next(); // '+' is redundant -> don't save

    if(!is->hasNext()) {errorCode = ParseError::UNEXPECTED_EOS; return false;} // An exponent requires at least one decimal digit

    // An exponent requires at least one decimal digit
    bool atLeastOneDigit = Json::isDecDigit(is->peek());

    while(is->hasNext() && Json::isDecDigit(is->peek())) {
        if(buf != nullptr) *buf += is->next();
        else is->next();
    }

    if(!atLeastOneDigit) errorCode = ParseError::MALFORMED_EXP;
    return atLeastOneDigit;
}

bool JsonTokenizer::readStr(String* buf) {
    while(is->hasNext()) {
        if(is->peek() == '\\') {
            is->next();
            if(!is->hasNext()) break; // Unterminated String. Break out of loop.
            else if(Json::isEscapeable(is->peek())) {
                if(buf != nullptr) *buf += Json::escape(is->next());
                else is->next();
            } else {
                errorCode = ParseError::UNESCAPEABLE_CHAR;
                return false;
            }
        } else if(is->peek() == '"') {
            is->next();
            return true;
        } else {
            if(buf != nullptr) *buf += is->next();
            else is->next();
        }
    }

    // String wasn't closed properly
    errorCode = ParseError::UNTERMINATED_STR;
    return false;
}

bool JsonTokenizer::matchStr(const char kw[], size_t length) {
    for(int i=0; i<length; i++) {
        if(!is->hasNext() || kw[i] != is->peek()) return false;
        else is->next();
    }
    return true;
}