#pragma once

namespace JStream {
    /** @brief Returns true if a character is a valid json whitespace **/
    inline bool isWhitespace(const char c) {
        return c == '\t' || c == '\n' || c == '\r' || c == ' ';
    }

    inline bool isNotWhitespace(const char c) {
        return c > 32;
    }

    /** @brief Returns true if a character is a valid json decimal digit **/
    inline bool isDecDigit(const char c) {
        return c >= 48 && c <= 57;
    }

    /** @brief Returns true if a character is a valid json decimal digit between (inclusive) 1 and 9 **/
    inline bool isOneNine(const char c) {
        return c >= 49 && c <= 57;
    }

    /** @brief Returns true if a character is a valid json hexadecimal digit **/
    inline bool isHexDigit(const char c) {
        //            0-9                     A-F                    a-f
        return (c>=48 && c <= 57) || (c >= 65 && c <= 70) || (c >= 97 && c <= 102);
    }

    /** @brief Returns true if a character is a valid digit in a json number **/
    inline bool isNumDigit(const char c) {
        return isDecDigit(c) || c == '-' || c == '+' || c == '.' || c == 'E' || c == 'e';
    }

    /** @brief Returns true if escaping a character results in a char valid in json. 
     * 
     * This function is designed to work with Json::escape. Since Json::escape can only escape single characters,
     * unicode characters (i.e. '\uXXXX') are excluded. They need to be handled seperately.
    */
    inline bool isEscapeable(const char c) {
        return c == '"' || c == '\\' || c == '/' || c == 'b' || c == 'f' || c == 'n' || c == 'r' || c == 't';
    }

    /** @brief Tries to escapes a character. Returns 0, if the char cannot be escaped. */
    char escape(const char c);

    /** @brief Parses a string to a long 
     * Skips leading whitespace, stops parsing at the first non-numeric char (except the sign '-')
     * 
     * @param defaultVal The value that is returned if not a single digit could be parsed
    */
    long stol(const char* str, long defaultVal=0);

    /** @brief Parses a string to a double 
     * Skips leading whitespace
     * 
     * @param defaultVal The value that is returned if not a single digit could be parsed
    */
    double stod(const char* str, double defaultVal=0.0);
}