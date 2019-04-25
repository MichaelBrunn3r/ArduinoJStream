cmake_minimum_required(VERSION 3.0)
project(ArduinoJStream)

add_subdirectory(test)

add_custom_target(createRelease
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}

    COMMAND rm -f ArduinoJStream.zip
    COMMAND cp -R src ArduinoJStream
    COMMAND cp LICENSE ArduinoJStream
    COMMAND zip -r ArduinoJStream.zip ArduinoJStream
    COMMAND rm -r ArduinoJStream
)