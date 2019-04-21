ARDUINO_TEST_HOST_DIR := Arduino/tests/host
REL_PATH_FROM_ARDUINO_TEST := ../../../

null :=
space := ${null} ${null}
${space} := ${space}

SRC_DIR := ../src
SRC_CPP_FILES := $(shell echo $(SRC_DIR)/*.cpp $(SRC_DIR)/*/*.cpp)

TEST_COMMON_DIR := ../lib/test-common
TEST_COMMON_CPP_FILES := $(shell echo $(TEST_COMMON_DIR)/*.cpp $(SRC_DIR)/*/*.cpp)

TEST_HOST_INCLUDE_DIRS := $(subst ${ },:,$(addprefix $(REL_PATH_FROM_ARDUINO_TEST),\
	$(SRC_DIR)\
	${TEST_COMMON_DIR}\
))

TEST_HOST_CPP_FILES := $(addprefix $(REL_PATH_FROM_ARDUINO_TEST),\
	$(SRC_CPP_FILES)\
	$(TEST_COMMON_CPP_FILES)\
	host/testJsonUtils.cpp\
	host/testParserImpl.cpp\
)

test_host:
	make -C $(ARDUINO_TEST_HOST_DIR) test ULIBDIRS=$(TEST_HOST_INCLUDE_DIRS) TEST_CPP_FILES="$(TEST_HOST_CPP_FILES)"

clean:
	make -C $(ARDUINO_TEST_HOST_DIR) clean

debug:
	gdb $(ARDUINO_TEST_HOST_DIR)/bin/host_tests