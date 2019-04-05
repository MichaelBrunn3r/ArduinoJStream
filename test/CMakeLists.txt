include_directories(${CMAKE_SOURCE_DIR}/test/MockArduino ${CMAKE_SOURCE_DIR}/lib/test ${CMAKE_SOURCE_DIR}/src)

file(GLOB_RECURSE ArduinoJStream_SRC "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE MockArduino_SRC "${CMAKE_SOURCE_DIR}/test/MockArduino/*.cpp" "${CMAKE_SOURCE_DIR}/lib/test/*.cpp")

add_library(Catch2 tests-main.cpp)
add_library(ArduinoJStream ${ArduinoJStream_SRC})
add_library(MockArduino ${MockArduino_SRC} ${Lib_Test_SRC})

add_executable(TestJsonParser TestJsonParser.cpp)
target_link_libraries(TestJsonParser Catch2 ArduinoJStream MockArduino)

add_executable(TestJsonUtils TestJsonUtils.cpp)
target_link_libraries(TestJsonUtils Catch2 ArduinoJStream MockArduino)

add_custom_target(testJsonParser ./TestJsonParser DEPENDS TestJsonParser)
add_custom_target(testJsonUtils ./TestJsonUtils DEPENDS TestJsonUtils)