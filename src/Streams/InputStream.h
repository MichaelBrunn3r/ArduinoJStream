#pragma once

class InputStream {
    public:
        virtual ~InputStream(){}
        /**
         * @brief Returns true if the stream has chars left
         */
        virtual bool hasNext() = 0;
        /**
         * @brief Returns the next char of the stream
         * 
         * @return char The next char of the stream, -1 if the stream ended
         */
        virtual char next() = 0;
        /**
         * @brief Returns the next char of the stream without advancing the stream
         * 
         * @return char The next char of the stream, -1 if the stream ended
         */
        virtual char peek() = 0;
        /**
         * @brief Set the maximum amount of time to wait for stream data
         * 
         * @param t The maximum timeout
         */
        virtual void setTimeout(unsigned long t) = 0;
};