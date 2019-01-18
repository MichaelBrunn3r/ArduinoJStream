#pragma once

#include <WString.h>
#include <Streams/InputStream.h>

namespace JStream {
    class JsonTokenizer {
        public:
            enum class Token : uint8_t {
                NaT = 0, // Not a Token
                OBJ_START, 
                OBJ_END, 
                ARR_START, 
                ARR_END,  
                COMMA, 
                STR, 
                FIELD_NAME,
                NUM,
                KW_NULL, 
                KW_TRUE, 
                KW_FALSE, 
                ERR
            };
            static const char* tokenTypeToStr(Token t);

            enum class ParseError : uint8_t {
                NaE = 0, // Not an Error
                UNTERMINATED_STR, 
                UNESCAPEABLE_CHAR, 
                UNEXPECTED_EOS,
                UNQUOTED_STR,
                MALFORMED_NUM
            };
            static const char* errorToStr(ParseError e);

            JsonTokenizer();
            JsonTokenizer(String str);
            JsonTokenizer(InputStream* is);
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
            inline bool hasNext() {return is->hasNext();}
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

            inline ParseError getErrorCode() {return errorCode;}
        private:
            InputStream* is = nullptr;
            Token currentToken = Token::NaT;
            String currentVal = "";

            ParseError errorCode = ParseError::NaE;

            void skipWhitespace() const;
            bool readNum(String* buf);
            bool readStr(String* buf);
            bool matchStr(const char kw[], size_t length);
    };
}