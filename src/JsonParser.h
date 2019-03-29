#pragma once

#include <Arduino.h>
#include <vector>
#include <Stream.h>
#include <Path.h>

namespace JStream {
    class JsonParser {
        public:
            JsonParser();
            JsonParser(Stream* stream);

            void parse(Stream* stream);
            /** @brief Returns true if the stream is at the closing '}'/']' of the current parent object/array */
            bool atEnd();
            /**
             * @brief Reads the stream until the start of the n-th succeeding array value
             * 
             * Stream position:
             * - on success: First char of the n-th value
             * - on fail: After closing ']' of the current array
             * 
             * Behaviour:
             * - Assumes the current parent collection is an array
             * - n=0, method returns immediately
             */
            bool nextVal(size_t n=1);
            /**
             * @brief Reads the stream until the next valid key in the current object and writes it into a buffer
             * 
             * Stream position:
             * - on success: First char of the value corresponding to the key
             * - on fail: After closing '}' of the current object
             * 
             * Behaviour:
             * - Assumes the current parent collection is an object
             * - Skips over invalid key-value pairs (e.g. ',"invalid" 1, "akey": 2')
             * - Skips over non key-value pairs (for example in arrays)
             * 
             * @param buf The buffer in which the key will be written. Set to NULL to not save the key.
             */
            bool nextKey(String* buf=NULL);
            /**
             * @brief Reads the stream until it finds the searched for key in the current object
             * 
             * Stream position:
             * - on success: First char of the value corresponding to the key
             * - on fail: After closing '}' of the current object
             * 
             * Behaviour:
             * - Treats the current parent collection as an object
             * - Skips over malformed invalid keys (e.g. ',"invalid" 1, "akey": 2'), including non strings (e.g ',1,2,3, "akey": 2]')
             *
             * @return true If the key was found in the current object(/array), false otherwise
             */
            bool findKey(const char* thekey);
            /** 
             * @brief Reads the stream until it finds the value at the given path. 
             * 
             * Stream position:
             * - on success: First char of the value corresponding to the key
             * - on fail: After closing '}' of the current object
             */
            bool find(Path& path);
            /** 
             * @brief Reads the stream until it finds the value at the given path. 
             * 
             * Stream position:
             * - on success: First char of the value corresponding to the key
             * - on fail: After closing '}' of the current object
             * 
             * Formatting:
             *  - "[..]": OFFSET segment, n-th element in a json array/object
             *  - "akey": KEY segment, child of a json object
             *  - "key1/key2[2]/key3": KEY segments are seperated from previous segments with a '/'
             *  - "akey[2][2]": OFFSET segments can be appended directly to previous segments 
             */
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
            bool exitCollection(size_t levels=1);
            /** @brief Skips the next object/array in the stream */
            bool skipCollection();
            /** 
             * @brief Reads over a string in the stream
             * @param inStr Indicates if the stream is positioned either inside or at the opening '"' of the string  
             **/
            bool skipString(bool inStr=false);
            /**
             * @brief Reads a string from the stream into a buffer
             * @param inStr Indicates if the stream is positioned either inside or at the opening '"' of the string 
             */
            bool readString(String& buf, bool inStr=false);
            /**
             * @brief Parses the next json integer
             * 
             * Skips leading Whitespaces, stops parsing at any non-numeric chars
             * 
             * @param defaultVal Default value, if not a single digit could be parsed
             */
            long parseInt(long defaultVal=0);
            /**
             * @brief Parses the next json number
             * 
             * Skips leading Whitespaces, stops parsing at any non-numeric chars
             * 
             * @param defaultVal Default value, if not a single digit could be parsed
             */
            double parseNum(double defaultVal=0.0);
            /**
             * @brief Parses the next json boolean
             * 
             * Skips leading Whitespaces, stops parsing at the first mismatched char (e.g. "falze": stops at 'z')
             * 
             * @param defaultVal Default value, if no valid boolean could be parsed
             */
            bool parseBool(bool defaultVal=false);
            /**
             * @brief Parses an array of integers
             * @param inArray Indicates the opening '[' was alread read
             */
            template <typename T>
            bool parseIntArray(std::vector<T>& vec, bool inArray=false);
            /**
             * @brief Parses an array of json numbers
             * @param inArray Indicates the opening '[' was alread read
             */
            bool parseNumArray(std::vector<double>& vec, bool inArray=false);
            /**
             * @brief Reads the stream until the first non-whitespace char
             * 
             * @return int The first non-whitespace in the stream, or -1 if the stream ended
             */
            int skipWhitespace();
        private:
            Stream* stream;

            /**
             * @brief Reads the stream until the start of the n-th succeeding key/value in the current object/array
             * 
             * Stream position:
             * - on success: First char of the n-th value
             * - on fail: After closing ']' of the current array
             * 
             * If n=0, method returns immediately.
             */
            bool next(size_t n=1);
    };
}