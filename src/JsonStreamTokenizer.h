#pragma once

#include <Arduino.h>
#include <WString.h>
#include <Stream.h>
#include <Streams/InputStream.h>
#include <WString.h>

class JsonStreamTokenizer {
    public:
        struct Token {
            enum class Type : byte {DOC_START, DOC_END, OBJ_START, OBJ_END, ARR_START, ARR_END, COLON, COMMA, KEY, NUM, STR, NUL, BOOL, TOKEN_ERROR, TOKEN_NULL};
            Type type;
            String val;

            static const char* typeToStr(Type t);
        };

        JsonStreamTokenizer();
        ~JsonStreamTokenizer();

        void tokenize(String str);
        void tokenize(InputStream* is);
        
        bool hasNext();
        Token peek();
        Token next();
    private:
        InputStream* is;
        Token current = Token{Token::Type::TOKEN_NULL, ""};
        bool isWhitespace(const char c) const;
        void skipWhitespace() const;
};