#include "JsonTokenizer.h"
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
            // Numbers are handled in special way, because they do not have a usefull prefix.
            // Calling 'is->next()' would unneccessarily complicate reading a Number.

            if (!readNum(buf)) return Token::ERR;
            else return Token::NUM;
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

const char* JsonTokenizer::tokenTypeToStr(Token t) {
    switch(t) {
        case Token::NaT: return "NaT";
        case Token::OBJ_START: return "{";
        case Token::OBJ_END: return "}";
        case Token::ARR_START: return "[";
        case Token::ARR_END: return "]";
        case Token::COMMA: return ",";
        case Token::STR: return "STR";
        case Token::FIELD_NAME: return "FIELD_NAME";
        case Token::NUM: return "NUM";
        case Token::KW_NULL: return "null";
        case Token::KW_TRUE: return "true";
        case Token::KW_FALSE: return "false";
        case Token::ERR: return "ERR";
        default: return "";
    }
}

const char* JsonTokenizer::errorToStr(ParseError e) {
    switch(e) {
        case ParseError::NaE: return "NaE";
        case ParseError::UNEXPECTED_EOS: return "UNEXPECTED_EOS"; 
        case ParseError::UNQUOTED_STR : return "UNQUOTED_STR";
        case ParseError::MALFORMED_NUM: return "MALFORMED_NUM";
        case ParseError::UNESCAPEABLE_CHAR: return "UNESCAPEABLE_CHAR";
        case ParseError::UNTERMINATED_STR: return "UNTERMINATED_STR";
        default: return "";
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

bool JsonTokenizer::readNum(String* buf) {
    /**
     * Deterministic Finite State Machine (DFSM) for validating Json numbers.
     * 
     *      |  START  | INT_SIGN |  [ZERO]  | [INT] | PERIOD | [DECIMAL] |    E     | EXP_SIGN | [EXP]
     * -----|---------|----------|----------|-------|--------|-----------|----------|----------|-------
     * 1..9 |  INT    |   INT    |    -     |  INT  | DECIMAL|  DECIMAL  |   EXP    |   EXP    |  EXP
     *   0  |  ZERO   |   ZERO   |    -     |  INT  | DECIMAL|  DECIMAL  |   EXP    |   EXP    |  EXP
     *   -  | INT_SIGN|    -     |    -     |   -   |   -    |     -     | EXP_SING |    -     |   -
     *   +  |    -    |    -     |    -     |   -   |   -    |     -     | EXP_SING |    -     |   -
     *   .  |    -    |    -     |  PERIOD  | PERIOD|   -    |     -     |    -     |    -     |   -
     *  e/E |    -    |    -     |    E     |   E   |   -    |     E     |    -     |    -     |   -
     * 
     * '[...]' - Accepting state
     * '-' - No transition for char -> Error
    **/

    enum FSM_State : uint8_t {
        // Accepting States
        INT = 0, ZERO = 1, DECIMAL = 2, EXP = 3,

        // Non-Accepting States
        START, INT_SIGN, PERIOD, E, EXP_SIGN, ERROR
    };
    #define isAcceptingState(x) (x <= 3)
    FSM_State state = FSM_State::START;

    while(is->hasNext()) {
        char c = is->peek();
        
        // State Machine Transitions sorted by their conditional char
        if(Json::isDecDigit(c)) {
            if(state == FSM_State::START || state == FSM_State::INT_SIGN) state = c == '0' ? FSM_State::ZERO : FSM_State::INT;
            else if(state == FSM_State::PERIOD) state = FSM_State::DECIMAL;
            else if(state == FSM_State::E || state == FSM_State::EXP_SIGN) state = FSM_State::EXP;
            else state = FSM_State::ERROR;
        } else if(c == '-') {
            if(state == FSM_State::START) state = FSM_State::INT_SIGN;
            else if(state == FSM_State::E) state = FSM_State::EXP_SIGN;
            else state = FSM_State::ERROR;
        } else if(c == '.') {
            if(state == FSM_State::INT || state == FSM_State::ZERO) state = FSM_State::PERIOD;
            else state = FSM_State::ERROR;
        } else if(c == 'e' || c == 'E') {
            if(state == FSM_State::INT || state == FSM_State::ZERO || state == FSM_State::DECIMAL) state = FSM_State::E;
            else state = FSM_State::ERROR;
        } else if(c == '+') {
            if(state == FSM_State::E) state = FSM_State::EXP_SIGN;
            else state = FSM_State::ERROR;
        } else {
            if(isAcceptingState(state)) break;
            else state = FSM_State::ERROR;
        }

        if(state == FSM_State::ERROR) break;

        if(state == FSM_State::INT || state == FSM_State::DECIMAL || state == FSM_State::EXP) {
            while(is->hasNext() && Json::isDecDigit(is->peek())) {
                char c = is->next();
                if(buf != nullptr) *buf += c;
            }
        } else {
            char c = is->next();
            if(buf != nullptr) *buf += c;        
        }
    }

    if(!isAcceptingState(state)) {
        errorCode = ParseError::MALFORMED_NUM;
        while(is->hasNext() && Json::isNumDigit(is->peek())) if(buf != nullptr) *buf += is->next();
        return false;
    } else return true;

    #undef isAcceptingState
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
    for(size_t i=0; i<length; i++) {
        if(!is->hasNext() || kw[i] != is->peek()) return false;
        else is->next();
    }
    return true;
}