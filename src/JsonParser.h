#pragma once

#include <vector>
#include <Stream.h>
#include <Path.h>
#include <limits>
#include <WString.h>

namespace JStream {
    class JsonParser {
        public:
            JsonParser();
            JsonParser(Stream& stream);

            void parse(Stream& stream);
            /** @brief Returns true if the stream is at the closing '}'/']' of the current parent object/array */
            bool atEnd();
            /**
             * @brief Reads the stream until the start of the n-th succeeding array value
             * 
             * Stream position:
             * - on success: First char of the n-th value
             * - on fail: At closing ']'/'}' of the current array/object
             * 
             * Behaviour:
             * - Assumes the current parent collection is an array
             * - n=0, method returns immediately
             */
            bool nextVal(size_t n=1);
            /**
             * @brief Returns true if a next valid key in the current json object exists and stores it in a buffer
             * 
             * Stream position:
             * - on success: First char of the value corresponding to the key
             * - on fail: At closing ']'/'}' of the current array/object
             * 
             * Behaviour:
             * - Assumes the current parent collection is an object
             * - Skips over invalid key-value pairs (e.g. ',"invalid" 1, "akey": 2')
             * - Skips over non key-value pairs (for example in arrays)
             * 
             * @param String* The Buffer in which the key will be stored (equals "" if no key exists)
             * @return true if a next key exists, false otherwise
             */
            bool nextKey(String* buf);
            /**
             * @brief Reads the stream until it finds the searched for key in the current object
             * 
             * Stream position:
             * - on success: First char of the value corresponding to the key
             * - on fail: At closing ']'/'}' of the current array/object
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
             * - on fail: At closing ']'/'}' of the current array/object
             */
            bool find(Path& path);
            /** 
             * @brief Reads the stream until it finds the value at the given path. 
             * 
             * Stream position:
             * - on success: First char of the value corresponding to the key
             * - on fail: At closing ']'/'}' of the current array/object
             * 
             * Formatting:
             *  - "[..]": OFFSET segment, n-th element in a json array/object
             *  - "akey": KEY segment, child of a json object
             *  - "key1/key2[2]/key3": KEY segments are seperated from previous segments with a '/'
             *  - "akey[2][2]": OFFSET segments can be appended directly to previous segments 
             */
            bool find(const char* path);
            /**
             * @brief Enters the immediatley following json array
             * Skips whitespace, fails if the next json element isn't beginning of an array
             */
            bool enterArr();
            /**
             * @brief Enters the immediatley following json object
             * Skips whitespace, fails if the next json element isn't beginning of an object
             */
            bool enterObj();
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
             * @brief Reads the stream until the first non-whitespace char
             * 
             * @return char The first non-whitespace character in the stream, or -1 if the stream ended
             */
            int skipWhitespace();
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
             * If T is an unsigned type, negative integers are ignored
             * @param inArray Indicates the opening '[' was alread read
             */
            template<typename T>
            bool parseIntArray(std::vector<T>& vec, bool inArray=false) {
                bool ignoreNeg = !std::numeric_limits<T>::is_signed; // Ignore negatiove integers if type is unsigned

                if(!inArray) {
                    int c = skipWhitespace();
                    if(c != '[') return false;
                    mStream->read();
                }

                T num = 0;
                T sign = 1;

                bool atLeastOneDigit = false;
                int c;
                do {
                    c = mStream->read();
                    switch(c) {
                        case ',':
                            if(atLeastOneDigit) {
                                vec.push_back(num*sign); // Save read number
                                num = 0; // Reset num
                                atLeastOneDigit = false;
                            }
                            sign = 1;

                            break;
                        case ']':
                            if(atLeastOneDigit) vec.push_back(num*sign);
                            return true;
                        case '-':
                            if(ignoreNeg) { // ignore/skip integer
                                int c;
                                do {
                                    c = mStream->read();
                                    if(c == ',') goto CASE_NEG_END;
                                    else if(c == ']') return true;
                                } while(c > 0);
                                return false;
                            } else if(!atLeastOneDigit) sign = -1;
                            CASE_NEG_END:
                            break;
                        case  '0': case  '1': case  '2': case  '3': case  '4': case  '5': case  '6': case  '7': case  '8': case  '9':
                            num = num*10 + c - '0';
                            atLeastOneDigit = true;
                            break;
                    }
                } while(c>0);

                return false;
            }
            /**
             * @brief Parses an array of json numbers
             * @param inArray Indicates the opening '[' was alread read
             */
            bool parseNumArray(std::vector<double>& vec, bool inArray=false);
        private:
            Stream* mStream = nullptr;

            /**
             * @brief Reads the stream until the start of the n-th succeeding key/value in the current object/array
             * 
             * Stream position:
             * - on success: First char of the n-th value
             * - on fail: At closing ']'/'}' of the current array/object
             * 
             * If n=0, method returns immediately.
             */
            bool next(size_t n=1);
    };
}