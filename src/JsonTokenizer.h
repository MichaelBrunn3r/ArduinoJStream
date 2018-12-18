#pragma once

#include <Arduino.h>
#include <WString.h>
#include <Stream.h>
#include <Streams/InputStream.h>
#include <WString.h>

class JsonTokenizer {
    public:
        enum class Token : byte {NaT, OBJ_START, OBJ_END, ARR_START, ARR_END, COLON, COMMA, KW_NULL, KW_TRUE, KW_FALSE, INT, FRAC, EXP, STR, ERR};
        static const char* tokenToStr(Token t);

        enum class ParseError : byte {NaE, MISSING_QUOTE, UNESCAPEABLE_CHAR, UNKNOWN_CHAR};

        JsonTokenizer();
        ~JsonTokenizer();

        /**
         * @brief Set the json string to tokenize
         * 
         * @param str The json string
         */
        void tokenize(String str);
        /**
         * @brief Set the input stream (json) to tokenize
         * 
         * @param is The input stream
         */
        void tokenize(InputStream* is);
        
        /**
         * @brief Returns true if the tokenizer has more tokens
         * 
         * @return true 
         * @return false 
         */
        bool hasNext();
        /**
         * @brief Returns the next token without advancing the tokenizer
         * 
         * If @param buf is a nullptr, the token value will not be returned (but still captured)
         * 
         * @param buf The buffer in which the value of the token will be stored
         * @return Token The token
         */
        Token peek(String* buf);
        /**
         * @brief Returns the next token from the json tokenizer
         * 
         * If @param buf is a nullptr, the token value will neither be returned nor captured
         * 
         * @param buf The buffer in which the value of the token will be stored
         * @return Token The token
         */
        Token next(String* buf);
    private:
        InputStream* is;
        Token currentToken = Token::NaT;
        String currentVal = "";

        ParseError errorCode = ParseError::NaE;

        void skipWhitespace() const;
        bool readInt(bool capture);
        bool readFrac(bool capture);
        bool readExp(bool capture);
        bool readStr(bool capture);
        bool matchStr(const char kw[], size_t length);
};