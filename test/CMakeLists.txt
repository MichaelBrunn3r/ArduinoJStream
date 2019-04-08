file(GLOB_RECURSE ArduinoJStream_SRC "${CMAKE_SOURCE_DIR}/src/*.cpp") # Src Files
file(GLOB_RECURSE MockArduino_SRC "${CMAKE_SOURCE_DIR}/test/MockArduino/*.cpp" "${CMAKE_SOURCE_DIR}/lib/test/*.cpp") # MockArduino Files

set(CMAKE_BUILD_TYPE DEBUG)

add_library(Catch2 tests-main.cpp)

add_library(ArduinoJStream ${ArduinoJStream_SRC})
target_include_directories(ArduinoJStream PUBLIC ${CMAKE_SOURCE_DIR}/src) # Src Headers
target_include_directories(ArduinoJStream PRIVATE ${CMAKE_SOURCE_DIR}/test/MockArduino ${CMAKE_SOURCE_DIR}/lib/test) # MockArduino Headers

add_library(MockArduino ${MockArduino_SRC} ${Lib_Test_SRC})
target_include_directories(MockArduino PRIVATE ${CMAKE_SOURCE_DIR}/test/MockArduino ${CMAKE_SOURCE_DIR}/lib/test) # MockArduino Headers

add_executable(TestJsonParser TestJsonParser.cpp)
target_link_libraries(TestJsonParser Catch2 ArduinoJStream MockArduino)
target_include_directories(TestJsonParser PUBLIC ${CMAKE_SOURCE_DIR}/src) # Src Headers
target_include_directories(TestJsonParser PRIVATE ${CMAKE_SOURCE_DIR}/test/MockArduino ${CMAKE_SOURCE_DIR}/lib/test) # MockArduino Headers

add_executable(TestJsonUtils TestJsonUtils.cpp)
target_link_libraries(TestJsonUtils Catch2 ArduinoJStream MockArduino)
target_include_directories(TestJsonUtils PUBLIC ${CMAKE_SOURCE_DIR}/src) # Src Headers
target_include_directories(TestJsonUtils PRIVATE ${CMAKE_SOURCE_DIR}/test/MockArduino ${CMAKE_SOURCE_DIR}/lib/test) # MockArduino Headers

add_custom_target(testJsonParser ./TestJsonParser DEPENDS TestJsonParser)
add_custom_target(testJsonUtils ./TestJsonUtils DEPENDS TestJsonUtils)