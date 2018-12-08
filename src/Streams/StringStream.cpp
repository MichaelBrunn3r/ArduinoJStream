#include "StringStream.h"

StringStream::StringStream(String str) : str(str) {}
StringStream::~StringStream() {}

bool StringStream::hasNext() {return idx < str.length();}

char StringStream::next() {return str.charAt(idx++);}

char StringStream::peek() {return str.charAt(idx);}

void StringStream::setTimeout(unsigned long t) {}