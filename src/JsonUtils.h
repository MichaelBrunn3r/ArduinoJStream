#pragma once

namespace Json {
    /** @brief Returns true if a character is a valid json whitespace **/
    bool isWhitespace(const char c) {
        return c == '\t' || c == '\n' || c == '\r' || c == ' ';
    }

    /** @brief Returns true if a character is a valid json decimal digit **/
    bool isDecDigit(const char c) {
        return c >= 48 && c <= 57;
    }

    /** @brief Returns true if a character is a valid json hexadecimal digit **/
    bool isHexDigit(const char c) {
        return c >= 65 && c <= 70 || c >= 97 && c <= 102;
    }

    /** @brief Returns true if a character is a valid digit in a json number **/
    bool isNumDigit(const char c) {
        return isDecDigit(c) || c == '-' || c == '+' || c == '.' || c == 'E' || c == 'e';
    }
}