file(GLOB_RECURSE ArduinoJStream_SRC "${CMAKE_SOURCE_DIR}/src/*.cpp") # Src Files
file(GLOB_RECURSE MockArduino_SRC "${CMAKE_SOURCE_DIR}/test/MockArduino/*.cpp" "${CMAKE_SOURCE_DIR}/lib/MockArduinoNative/*.cpp")

set(CMAKE_BUILD_TYPE DEBUG)

#############
# Libraries #
#############

add_library(Catch2 tests-main.cpp)

add_library(MockArduino ${MockArduino_SRC})
target_include_directories(MockArduino PUBLIC ${CMAKE_SOURCE_DIR}/test/MockArduino ${CMAKE_SOURCE_DIR}/lib/MockArduinoNative) # MockArduino Headers

add_library(ArduinoJStream ${ArduinoJStream_SRC})
target_include_directories(ArduinoJStream PUBLIC ${CMAKE_SOURCE_DIR}/src) # Src Headers
target_include_directories(ArduinoJStream PUBLIC ${CMAKE_SOURCE_DIR}/test/MockArduino ${CMAKE_SOURCE_DIR}/lib/MockArduinoNative) # MockArduino Headers

###############
# Executables #
###############

add_executable(TestParserImpl TestParserImpl.cpp)
target_link_libraries(TestParserImpl MockArduino Catch2 ArduinoJStream)

add_executable(TestParserNav TestParserNav.cpp)
target_link_libraries(TestParserNav MockArduino Catch2 ArduinoJStream)

add_executable(TestJsonUtils TestJsonUtils.cpp)
target_link_libraries(TestJsonUtils MockArduino Catch2 ArduinoJStream)

###########
# Targets #
###########

add_custom_target(runTestParserImpl COMMAND ./TestParserImpl DEPENDS TestParserImpl)
add_custom_target(runTestParserNav COMMAND ./TestParserNav DEPENDS TestParserNav)
add_custom_target(runTestJsonUtils COMMAND ./TestJsonUtils DEPENDS TestJsonUtils)
add_custom_target(runTestAll COMMAND make runTestParserImpl && make runTestParserNav && make runTestJsonUtils)