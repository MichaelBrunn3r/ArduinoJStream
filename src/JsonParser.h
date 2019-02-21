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
             * - on fail: Closing ']' of the current array
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
             * - on fail: Closing '}' of the current object
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
             * - on fail: Stream is read until the end of the current object
             * 
             * Behaviour:
             * - Treats the current parent collection as an object
             * - Skips over malformed invalid keys (e.g. ',"invalid" 1, "akey": 2'), including non strings (e.g ',1,2,3, "akey": 2]')
             *
             * @return true If the key was found in the current object(/array), false otherwise
             */
            bool findKey(const char* thekey);
            /** @brief Reads the stream until it finds the value at the given path. */
            bool find(Path& path);
            /** 
             * @brief Reads the stream until it finds the value at the given path. 
             * 
             * Formatting:
             *  - "[..]": OFFSET segment, n-th element in a json array/object
             *  - "akey": KEY segment, child of a json object
             *  - "key1/key2[2]/key3": KEY segments are seperated from other segments with a '/'
             *  - "akey[2][2]": OFFSET segments can be appended directly to other segments 
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

            bool parseInt(long& num);
            long parseInt();

            /**
             * @brief Parses an array of integers
             * @param inArray Indicates the opening '[' was alread read
             */
            template <typename T>
            bool parseIntArray(std::vector<T>& vec, bool inArray=false);
            
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
    };
}