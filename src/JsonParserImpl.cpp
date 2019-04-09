#include "JsonParser.h"
#include <Internals/JsonUtils.h>
#include <Internals/NumAccumulator.h>

namespace JStream {
    JsonParser::JsonParser() {}
    JsonParser::JsonParser(Stream* stream) : stream(stream) {}

    void JsonParser::parse(Stream* stream) {
        this->stream = stream;
    }

    bool JsonParser::atEnd() {
        int c = skipWhitespace();
        return c!=0 && c!=-1 && c != '}' && c != ']';
    }

    bool JsonParser::readString(String& buf, bool inStr) {
        if(!inStr) {
            int c = skipWhitespace();
            if(c != '"') return false; 
            stream->read(); // Read opening '"'
        }

        int c = stream->read();
        while(c != 0 && c != -1) {
            if(c == '\\') {
                c = Internals::escape(stream->read());
                if(c == 0) break;
            } else if (c == '"') return true;

            buf += (char)c;
            c = stream->read();
        }

        return false; // Stream ended without closing the string
    }
    
    int JsonParser::strcmp(const char* cstr, bool inStr) {
        if(!inStr) {
            skipWhitespace();
            if(stream->read() != '"') return false; // Read opening '"'
        }

        int c = -1;
        while(*cstr) {
            c = stream->read();

            if(c == '\\') {
                if(!(c = Internals::escape(stream->read()))) {
                    skipString(true);
                    return 1; // char is unescapeable -> any string is bigger than an incorrect one
                }
            } else if(c == '"') return 1; // stream_stream ended but cstr didn't -> cstr > stream_str

            if(*cstr != c) {
                skipString(true);
                return *cstr > c ? 1 : -1;
            }

            cstr++;
        }

        c = stream->read();
        if(c == '"') return 0;
        else if(c == -1 || c == 0) return 1;
        else {
            skipString(true);
            return -1;
        }
    }

    long JsonParser::parseInt(long defaultVal) {
        long result = 0;    
        long sign = 1;
        
        int c = skipWhitespace();
        if(c == '-') {
            stream->read();
            c = stream->peek();
            sign = -1;
        }

        bool moreThanOneDigit = false;
        while(c>0) {
            switch(c) {
                case  '0': case  '1': case  '2': case  '3': case  '4': case  '5': case  '6': case  '7': case  '8': case  '9':
                    result = result*10 + stream->read() - '0';
                    moreThanOneDigit = true;
                    break;
                default:
                    goto END_PARSING;
            }
            c = stream->peek();
        }
        END_PARSING:

        if(!moreThanOneDigit) return defaultVal;
        
        return result*sign;
    }    

    double JsonParser::parseNum(double defaultVal) {
        Internals::NumAccumulator acc;

        // Determine number sign
        int c = skipWhitespace();
        if(c == '-') {
            stream->read();
            c = stream->peek();
            acc.sign = -1;
        }

        // Parse number
        while(c = stream->peek()) {
            switch(c) {
                case  '0': case  '1': case  '2': case  '3': case  '4': case  '5': case  '6': case  '7': case  '8': case  '9':
                    acc.addDigitToSegment(stream->read() - '0');
                    break;
                case '.':
                    if(!acc.segmentHasAtLeastOneDigit) return defaultVal; // Check if prev segment has >=1 digits

                    stream->read();
                    acc.setSegment(Internals::NumAccumulator::NumSegment::DECIMAL);
                    break;
                case 'e': case 'E':
                    if(!acc.segmentHasAtLeastOneDigit) return defaultVal; // Check if prev segment has >=1 digits

                    stream->read();
                    acc.setSegment(Internals::NumAccumulator::NumSegment::EXPONENT);

                    c = skipWhitespace();

                    if(c == '+') stream->read();
                    else if(c == '-') {
                        stream->read();
                        acc.expSign = -1;
                    }
                    break;
                default:
                    goto END_PARSING;
            }
        }
        END_PARSING:

        if(!acc.segmentHasAtLeastOneDigit) return defaultVal;
        
        return acc.get();
    }

    bool JsonParser::parseBool(bool defaultVal) {
        static const char* str_true = "true";
        static const char* str_false = "false";

        bool result;
        char* compareTo;
        int c = skipWhitespace();
        if(c == 't') {
            result = true;
            compareTo = (char*)str_true + 1;
        }
        else if(c == 'f') {
            result = false;
            compareTo = (char*)str_false + 1;
        }
        else return defaultVal;

        stream->read();

        while(*compareTo) {
            if(stream->peek() != *compareTo) return defaultVal;
            stream->read();
            compareTo++;
        }

        return result;
    }

    template<typename T>
    bool JsonParser::parseIntArray(std::vector<T>& vec, bool inArray) {
        if(!inArray) {
            int c = skipWhitespace();

            if(c != '[') return false;
            stream->read();
        }

        T num = 0;
        T sign = 1;

        bool moreThanOneDigits = false;
        int c;
        do {
            c = stream->read();
            switch(c) {
                case ',':
                    if(moreThanOneDigits) {
                        vec.push_back(num*sign); // Save read number
                        num = 0; // Reset num
                        moreThanOneDigits = false;
                    }
                    sign = 1;

                    break;
                case ']':
                    if(moreThanOneDigits) vec.push_back(num*sign);
                    return true;
                case '-':
                    if(!moreThanOneDigits) sign = -1;
                    break;
                case  '0': case  '1': case  '2': case  '3': case  '4': case  '5': case  '6': case  '7': case  '8': case  '9':
                    num = num*10 + c - '0';
                    moreThanOneDigits = true;
                    break;
            }
        } while(c>0);

        return false;
    }

    template bool JsonParser::parseIntArray<byte>(std::vector<byte>& vec, bool inArray);
    template bool JsonParser::parseIntArray<char>(std::vector<char>& vec, bool inArray);
    template bool JsonParser::parseIntArray<long>(std::vector<long>& vec, bool inArray);

    bool JsonParser::parseNumArray(std::vector<double>& vec, bool inArray) {
        if(!inArray) {
            int c = skipWhitespace();

            if(c != '[') return false;
            stream->read();
        }

        Internals::NumAccumulator acc;
        int c;
        do {
            c = stream->read();
            switch(c) {
                case ',':
                    if(acc.segmentHasAtLeastOneDigit) vec.push_back(acc.get()); // Save read number
                    acc.reset();
                    break;
                case ']':
                    if(acc.segmentHasAtLeastOneDigit) vec.push_back(acc.get()); // Save read number
                    return true;
                case '-':
                    if(!acc.segmentHasAtLeastOneDigit) {
                        if(acc.currentSegment == Internals::NumAccumulator::PRE_DECIMAL) acc.sign = -1;
                        else if(acc.currentSegment == Internals::NumAccumulator::EXPONENT) acc.expSign = -1;
                    }
                    break;
                case  '0': case  '1': case  '2': case  '3': case  '4': case  '5': case  '6': case  '7': case  '8': case  '9':
                    acc.addDigitToSegment(c - '0');
                    break;
                case '.':
                    if(!acc.segmentHasAtLeastOneDigit) {
                        acc.reset();
                        if(!next()) return true;
                    }
                    acc.setSegment(Internals::NumAccumulator::NumSegment::DECIMAL);
                    break;
                case 'e': case 'E':
                    if(!acc.segmentHasAtLeastOneDigit) {
                        acc.reset();
                        if(!next()) return true;
                    }

                    acc.setSegment(Internals::NumAccumulator::NumSegment::EXPONENT);
                    break;
            }
        } while(c>0);

        return false;
    }
} // JStream
