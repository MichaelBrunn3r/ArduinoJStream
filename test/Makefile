# define ' ' char (used by $(subst ...))
null :=
space := ${null} ${null}
${space} := ${space}

# Directories
ARDUINO_TEST-ON-HOST_DIR := Arduino/tests/host# Path to the Arduino test-on-host environment
ARDUINO_TEST-ON-HOST_DIR_INV := ../../../# Path from the Arduino test-on-host environment to this file
SRC_DIR := ../src
LIB_DIRS := ../lib/ArduinoTestUtils

# Header files
TEST-ON-HOST_INCLUDE_DIRS := $(subst ${space},:,$(addprefix $(ARDUINO_TEST-ON-HOST_DIR_INV),\
	$(SRC_DIR)\
	$(addsuffix /src, ${LIB_DIRS})\
))

# Implementation files
SRC_CPP_FILES := $(shell echo $(SRC_DIR)/*.cpp $(SRC_DIR)/*/*.cpp)
LIB_CPP_FILES := $(foreach dir, $(LIB_DIRS), $(shell echo $(dir)/src/*.cpp))

# test-on-host parameters
TEST-ON-HOST_CPP_FILES := $(addprefix $(ARDUINO_TEST-ON-HOST_DIR_INV),\
	$(SRC_CPP_FILES)\
	$(LIB_CPP_FILES)\
	host/testJsonUtils.cpp\
	host/testParserImpl.cpp\
	host/testParserNav.cpp\
)
TEST-ON-HOST_OPTZ ?= -O0

###################
# build/run tests #
###################

.PHONY: test-on-host
test-on-host:
	make -C $(ARDUINO_TEST-ON-HOST_DIR) test ULIBDIRS=$(TEST-ON-HOST_INCLUDE_DIRS) TEST_CPP_FILES="$(TEST-ON-HOST_CPP_FILES)" OPTZ=$(TEST-ON-HOST_OPTZ)

.PHONY: debug
debug:
	gdb $(ARDUINO_TEST-ON-HOST_DIR)/bin/host_tests

############
# cleaning #
############

.PHONY: clean-test
clean-test:
	- rm host/*o host/*.gcno host/*.gcda

.PHONY: clean-all
clean-all: clean-test
	- rm $(SRC_DIR)/*o  $(SRC_DIR)/*/*o $(SRC_DIR)/*.gcno $(SRC_DIR)/*/*.gcno $(SRC_DIR)/*.gcda $(SRC_DIR)/*/*.gcda
	- rm $(TEST_COMMON_DIR)/*o $(TEST_COMMON_DIR)/*.gcno $(TEST_COMMON_DIR)/*.gcda