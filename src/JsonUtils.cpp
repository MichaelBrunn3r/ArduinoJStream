#include "JsonUtils.h"

namespace JStream {
    char escape(const char c) {
        switch(c) {
            // chars that stay unchanged
            /** case '"': return '"';
             *  case '\\': return '\\';
             *  case '/': return '/'; 
             **/

            // sorted by suspected frequency
            case 'n': return '\n';
            case 't': return '\t';
            case 'r': return '\r';
            case 'b': return '\b';
            case 'f': return '\f';
            default: return c;
        }
    }
}