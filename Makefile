SRC := src
TEST_SRC := test
LIB_SRC := lib
BIN := bin

SRC_EXCLUDE := $(SRC)/main.cpp
SRC_FILES := $(filter-out $(SRC_EXCLUDE), $(wildcard $(SRC)/*.cpp) $(wildcard $(SRC)/**/*.cpp)) 
SRC_OBJ_FILES := $(patsubst %.cpp, $(BIN)/%.o, $(SRC_FILES))

TEST_SRC_EXCLUDE := $(wildcard $(TEST_SRC)/Test*)
TEST_SRC_FILES := $(filter-out $(TEST_SRC_EXCLUDE), $(wildcard $(TEST_SRC)/*.cpp) $(wildcard $(TEST_SRC)/**/*.cpp) $(wildcard $(LIB_SRC)/**/*.cpp))
TEST_OBJ_FILES := $(patsubst %.cpp, $(BIN)/%.o, $(TEST_SRC_FILES))

test: testJsonTokenizer testJsonParser
	
testJsonTokenizer: $(TEST_SRC)/catch.hpp Makefile $(BIN)/TestJsonTokenizer
	$(BIN)/TestJsonTokenizer

testJsonParser: $(TEST_SRC)/catch.hpp Makefile $(BIN)/TestJsonParser
	$(BIN)/TestJsonParser

clean:
	- rm -r $(BIN) &>/dev/null

# Compiles c++ files to object files and puts them in the /bin subfolder
$(BIN)/%.o : %.cpp
	- mkdir -p $(@D)
	g++ -c -o $@ $< -I$(SRC) -I$(TEST_SRC)/MockArduino -ggdb3

$(BIN)/TestJsonTokenizer : $(TEST_SRC)/TestJsonTokenizer.cpp $(SRC_OBJ_FILES) $(TEST_OBJ_FILES)
	g++ -o $@ $^ -I$(SRC) -I$(TEST_SRC)/MockArduino -ggdb3

$(BIN)/TestJsonParser : $(TEST_SRC)/TestJsonParser.cpp $(SRC_OBJ_FILES) $(TEST_OBJ_FILES)
	g++ -o $@ $^ -I$(SRC) -I$(TEST_SRC)/MockArduino -I$(LIB_SRC)/test -ggdb3