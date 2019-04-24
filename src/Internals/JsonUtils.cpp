#include "JsonUtils.h"
#include <Internals/NumAccumulator.h>

namespace JStream {
    namespace Internals {
        unsigned char escape(const unsigned char c) {
            switch(c) {
                case '"': return '"';
                case '\\': return '\\';
                case '/': return '/'; 
                case 'n': return '\n';
                case 't': return '\t';
                case 'r': return '\r';
                case 'b': return '\b';
                case 'f': return '\f';
                default: return 0;
            }
        }

        long stol(const char* str, long defaultVal) {
            long result = 0;    
            long sign = 1;

            while(isWhitespace(static_cast<unsigned char>(*str))) str++; // Skip whitespace

            if(static_cast<unsigned char>(*str) == '-') {
                str++;
                sign = -1;
            }

            bool atLeastOneDigit = false;
            while(static_cast<unsigned char>(*str)) {
                switch(static_cast<unsigned char>(*str)) {
                    case  '0': case  '1': case  '2': case  '3': case  '4': case  '5': case  '6': case  '7': case  '8': case  '9':
                        result = result*10 + static_cast<unsigned char>(*str++) - '0';
                        atLeastOneDigit = true;
                        break;
                    default:
                        goto END_PARSING;
                }
            }
            END_PARSING:

            if(!atLeastOneDigit) return defaultVal;
            
            return result*sign;
        }

        double stod(const char* str, double defaultVal) {
            NumAccumulator acc;

            while(isWhitespace(static_cast<unsigned char>(*str))) str++; // Skip whitespace

            // Determine number sign
            if(static_cast<unsigned char>(*str) == '-') {
                str++;
                acc.sign = -1;
            }

            // Parse number
            while(static_cast<unsigned char>(*str)) {
                switch(*str) {
                    case  '0': case  '1': case  '2': case  '3': case  '4': case  '5': case  '6': case  '7': case  '8': case  '9':
                        acc.addDigitToSegment(static_cast<unsigned char>(*str++) - '0');
                        break;
                    case '.':
                        if(!acc.segmentHasAtLeastOneDigit) return defaultVal; // Check if prev segment has >=1 digits

                        str++;
                        acc.setSegment(Internals::NumAccumulator::NumSegment::DECIMAL);
                        break;
                    case 'e': case 'E':
                        if(!acc.segmentHasAtLeastOneDigit) return defaultVal; // Check if prev segment has >=1 digits

                        str++;
                        acc.setSegment(Internals::NumAccumulator::NumSegment::EXPONENT);

                        while(isWhitespace(static_cast<unsigned char>(*str))) str++; // Skip whitespace

                        if(static_cast<unsigned char>(*str) == '+') str++;
                        else if(static_cast<unsigned char>(*str) == '-') {
                            str++;
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
    } // Internals
}