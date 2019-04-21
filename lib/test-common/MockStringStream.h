#pragma once

#include <Stream.h>
#include <Arduino.h>
#include <WString.h>

namespace MockArduino {
    namespace Native {
        class MockStringStream : public Stream {
            public:
                MockStringStream();
                MockStringStream(String str);
                MockStringStream(const char* cstr);

                // Stream Implementation
                int available();
                int read();
                int peek();
                size_t write(uint8_t);

                void setString(String str);
                String peekString();

                MockStringStream& operator= (const MockStringStream& other); 

            private:
                String mStr;
                const char* mCstr;
        };    
    } // Native
} // MockArduino
