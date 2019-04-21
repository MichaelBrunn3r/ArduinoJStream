#include "MockStringStream.h"
#include <cstring>

namespace MockArduino {
    namespace Native {
        //////////////////
        // Constructors //
        //////////////////

        MockStringStream::MockStringStream() : MockStringStream("") {}

        MockStringStream::MockStringStream(String str) {
            setString(str);
        }

        MockStringStream::MockStringStream(const char* cstr) {
            mStr = String(cstr);
            mCstr = mStr.c_str();
        }

        ///////////////////////////
        // Stream Implementation //
        ///////////////////////////

        int MockStringStream::available() {
            if(*mCstr == 0) return 0;
            long charsRead = mCstr - mStr.c_str(); 
            return mStr.length() - charsRead;
        }

        int MockStringStream::read() {
            if(available() > 0) {
                return *mCstr++;
            } else return -1;
        }

        int MockStringStream::peek() {    
            return *mCstr;
        }

        size_t MockStringStream::write(uint8_t) {
            return 1;
        }

        /////////////
        // Methods //
        /////////////

        void MockStringStream::setString(String str) {
            mStr = str;
            mCstr = mStr.c_str();
        }

        String MockStringStream::peekString() {
            return String(mCstr);
        }

        ///////////////
        // Operators //
        ///////////////

        MockStringStream& MockStringStream::operator= (const MockStringStream& other) {
            mStr = other.mStr;
            mCstr = mStr.c_str();
            return *this;
        }
    } // Native
} // MockArduino
