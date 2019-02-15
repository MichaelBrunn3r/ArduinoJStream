#pragma once

#include <Arduino.h>
#include <vector>
#include <Stream.h>

namespace JStream {
    class JsonParser {
        public:
            JsonParser();
            JsonParser(Stream* stream);

            void parse(Stream* stream);
            /**
             * @brief Returns true if the stream is at the closing '}'/']' of the current json array/object
             */
            bool atEnd();
            /**
             * @brief Reads the stream until the start of the n-th succeeding array value.
             *
             * If n=0, method returns immediately.
             * This method stops reading at the first char of the value (e.g. stops at 1 -> "...,    1337]").
             * If it comes across the closing ']' of the current array, no next value exists and it stops reading.
             */
            bool nextVal(size_t n=1);
            /**
             * @brief Reads the stream until the start of the value of the n-th succeeding key-value pair in the current object
             * and returns its key.
             * 
             * If n=0, method returns immediately.
             * This method stops reading at the first char of the value (e.g. stops at 1 -> "..., 'akey':  1337]").
             * If it comes across the closing '}' of the current object, it stops reading, since no next value exists.
             * 
             * @return String The name of the n-th succeeding key, or "" if it doesn't exist
             */
            String nextKey(size_t n=1);
            /**
             * @brief Reads the stream until it finds the searched for key in the current object
             * 
             * This method stops reading at the first char of the value (e.g. stops at 1 -> "..., 'akey':  1337]").
             * If it comes across the closing '}' of the current object, the key couldn't be found, and it stops reading.
             * Skips over empty key-value pairs (e.g. "{'1': 1,, '2'; 2}") and malformed keys (e.g. "{'1': 1, 'notakey' 2}" or "{'1': 1, 2, '3': 3}").
             * 
             * @return true If the key was found in the current object
             * @return false If the key couldn't be found in the current object or the stream ended
             */
            bool findKey(const char* thekey);
            bool find(const char* path);
            /**
             * @brief Exits the specified number of parent objects/arrays
             * 
             * - levels=0: returns immediately
             * - levels=1: method exits only the current object/array
             * - levels=1+x: method exits current object/array and x of its parents
             * 
             * @param levels The number of parent objects/arrays to exit
             */
            bool exit(size_t levels=1);

            

            template <typename T>
            void toArray(std::vector<T>& vec);
        private:
            Stream* stream;

            /**
             * @brief Reads the stream until the start of the n-th succeeding key/value in the current object/array
             * 
             * If n=0, method returns immediately.
             * This method stops reading at the first char of the key/value (keys allways begin with '"').
             * If it comes across the closing '}'/']' of the current object/array, it stops reading, since no next key/value exists.
             * 
             * @return true if the n-th succeeding key/value was found
             */
            bool next(size_t n=1);
            void skipWhitespace();
            /** 
             * @brief Reads the stream until coming across the closing '"' 
             * 
             * Assumes the opening '"' was already read
             **/
            void exitString();
            /**
             * @brief Reads a string from the stream until coming across the closing '"'
             * 
             * Assumes the opening '"' was already read
             */
            String readString();
    };
}