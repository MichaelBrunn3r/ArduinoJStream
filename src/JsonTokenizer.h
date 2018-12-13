#pragma once

#include <Arduino.h>
#include <WString.h>
#include <Stream.h>
#include <Streams/InputStream.h>
#include <WString.h>

class JsonTokenizer {
    public:
        enum class Token : byte {NONE, OBJ_START, OBJ_END, ARR_START, ARR_END, COLON, COMMA, KW_NULL, KW_TRUE, KW_FALSE, NUM, STR, ERROR};
        static const char* tokenToStr(Token t);

        JsonTokenizer();
        ~JsonTokenizer();

        void tokenize(String str);
        void tokenize(InputStream* is);
        
        bool hasNext();
        Token peek(String* buf);
        Token next(String* buf);
    private:
        InputStream* is;
        Token currentToken = Token::NONE;
        String currentVal = "";

        void skipWhitespace() const;
        void readNum(bool capture);
        void readStr(bool capture);
        bool matchStr(const char kw[]);
};