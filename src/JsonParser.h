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
             * @brief Returns the next valid key in the current json object
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
             * @return String The key
             */
            String nextKey();
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
             * @brief Enters the immediatley following json collection
             * 
             * Skips whitespace, fails if the immediate following json element isn't a collection or eof
             * 
             * @return char "{" if the stream entered an object, "[" if it entered an array, 0 otherwise
             */
            char enterCollection();
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
             * @param inStr Indicates whether the stream is positioned inside the string or before the opening '"'
             **/
            bool skipString(bool inStr=false);
            /**
             * @brief Reads a string from the stream into a buffer
             * @param inStr Indicates whether the stream is positioned inside the string or before the opening '"'
             */
            bool readString(String& buf, bool inStr=false);
            /**
             * @brief Compares a string with the immediate next string in the stream
             * @param cstr C string to be compared
             * @param inStr Indicates whether the stream is positioned inside the string or before the opening '"'
             * @return -1: cstr<stream_str, 0: cstr==stream_str, 1: cstr>stream_str or stream_str is invalid
             */
            int strcmp(const char* cstr, bool inStr=false);
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
             * @return char The first non-whitespace character in the stream, or -1 if the stream ended
             */
            char skipWhitespace();
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