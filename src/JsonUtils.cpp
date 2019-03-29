#include "JsonUtils.h"

namespace JStream {
    char escape(const char c) {
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

        while(isWhitespace(*str)) str++; // Skip whitespace

        if(*str == '-') {
            str++;
            sign = -1;
        }

        bool atLeastOneDigit = false;
        while(*str) {
            switch(*str) {
                case  '0': case  '1': case  '2': case  '3': case  '4': case  '5': case  '6': case  '7': case  '8': case  '9':
                    result = result*10 + *str++ - '0';
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
}