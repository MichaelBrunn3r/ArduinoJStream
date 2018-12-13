#pragma once

/** @brief Returns true if a character is a valid json whitespace **/
bool isWhitespace(const char c) {
    return c == '\t' || c == '\n' || c == '\r' || c == ' ';
}

/** @brief Returns true if a character is a valid json decimal digit **/
bool isDecDigit(const char c) {
    return c >= 48 && c <= 57;
}

/** @brief Returns true if a character is a valid prefix of a json number **/
inline bool isNumStart(const char c) {
    return isDecDigit(c) || c == '-';
}

/** @brief Returns true if a character is a valid json hexadecimal digit **/
bool isHexDigit(const char c) {
    return c >= 65 && c <= 70 || c >= 97 && c <= 102;
}

/** @brief Returns true if a character is a valid json number digit **/
bool isNumDigit(const char c) {
    return isDecDigit(c) || c == '-' || c == '+' || c == '.' || c == 'E' || c == 'e';
}