#pragma once

#include <limits.h>

namespace JStream {
    namespace Internals {
        /**
         * @brief Tool used to parse a json number in a stream by accumulating its value 
         * 
         * This class accelerates the parsing of json numbers by utilizing an int as a buffer:
         *  - Parses the number to an int buffer "buf" (ignores decimal point) and remembers the number of decimal places "d"
         *  - Adjusts decimal placee: d = d - exponent (in scientific notation)
         *  - Calculates result: result = sign * ((double) buf) * 10^d
         */
        class NumAccumulator {
             public:
                enum NumSegment {PRE_DECIMAL, DECIMAL, EXPONENT};

                double sign = 1;
                unsigned long expSign = 1;
                bool segmentHasAtLeastOneDigit = false; // Indicates if the current segment has at least 1 digit (i.e. is defined)

            private:
                double result = 0.0;
                unsigned long exponent = 0; // The exponent of the double (e.g. "1.2e-123": exponent = -123)
                unsigned long buf = 0; // The int buffer
                static const unsigned long buf_max = (ULONG_MAX-9)/10; // The maximum value the buffer can take
                unsigned long tens_adjustment = 1;
                long decimalPlaces = 0;
                NumSegment currentSegment = PRE_DECIMAL;

            public:
                void setSegment(NumSegment seg) {
                    currentSegment = seg;
                    segmentHasAtLeastOneDigit = false;
                }

                void addDigitToSegment(unsigned int digit) {
                    segmentHasAtLeastOneDigit = true;

                    switch(currentSegment) {
                        case DECIMAL:
                            decimalPlaces++;
                        case PRE_DECIMAL:
                            // Prevent overflow of the buffer
                            if(buf > buf_max ) {
                                if(result > 0) result *= tens_adjustment;
                                result += buf;
                                tens_adjustment = 1;
                                buf = 0;
                            }

                            buf = buf*10 + digit;
                            tens_adjustment *= 10;
                            break;
                        case EXPONENT:
                            exponent = exponent*10 + digit;
                    }
                }

                double get() {
                    result *= tens_adjustment;
                    result += buf;

                    decimalPlaces -= exponent * expSign;

                    if(decimalPlaces > 0) for(int i=0; i<decimalPlaces; i++) result *= 0.1;
                    else if(decimalPlaces < 0) for(int i=0; i>decimalPlaces; i--) result *= 10;

                    return result * sign;
                }

                void reset() {
                    sign = 1;
                    expSign = 1;
                    segmentHasAtLeastOneDigit = false;

                    result = 0.0;
                    exponent = 0;
                    buf = 0;
                    tens_adjustment = 1;
                    decimalPlaces = 0;
                    currentSegment = PRE_DECIMAL;
                }
        };
    }
}