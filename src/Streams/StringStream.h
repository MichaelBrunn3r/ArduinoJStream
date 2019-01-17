#pragma once
#include <WString.h>
#include <Streams/InputStream.h>

namespace JStream {
    class StringStream : public InputStream {
        public:
            StringStream(String str);
            ~StringStream();
            bool hasNext();
            char next();
            char peek();
            void setTimeout(unsigned long t);
        private:
            String str;
            unsigned int idx = 0;
    };
}