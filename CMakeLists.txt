cmake_minimum_required(VERSION 3.0)
project(ArduinoJStream)

add_subdirectory(test)

add_custom_target(updateMockArduino
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/test
    
    COMMAND rm -R -f MockArduino
    COMMAND rm -R -f ${CMAKE_SOURCE_DIR}/lib/MockArduinoNative

    COMMAND curl -s https://api.github.com/repos/MichaelBrunn3r/MockArduino/releases/latest 
                | grep "browser_download_url" 
                | cut -d : -f 2,3 
                | tr -d \\\" | wget -qi -
    COMMAND unzip MockArduino.zip
    COMMAND rm MockArduino.zip
    
    COMMAND mv MockArduino/native MockArduino/MockArduinoNative
    COMMAND mv MockArduino/MockArduinoNative ${CMAKE_SOURCE_DIR}/lib
)

add_custom_target(createRelease
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}

    COMMAND rm -f ArduinoJStream.zip
    COMMAND cp -R src ArduinoJStream
    COMMAND cp LICENSE ArduinoJStream
    COMMAND zip -r ArduinoJStream.zip ArduinoJStream
    COMMAND rm -r ArduinoJStream
)