#
# rv64-emu -- Simple 64-bit RISC-V simulator
#
# Copyright (C) 2016,2019  Leiden University, The Netherlands.
#

CXX = c++

CXXFLAGS = -std=c++17 -Wall -Weffc++ -g 
LDFLAGS = -lstdc++fs

SRC_DIR = ./src
OBJ_DIR = ./obj

SRC_FILES = $(wildcard ${SRC_DIR}/*.cc) \
			$(wildcard ${SRC_DIR}/stages/*.cc)
OBJ_FILES = $(patsubst %.cc,${OBJ_DIR}/%.o,$(notdir ${SRC_FILES}))

TARGET = rv64-emu

${TARGET}: $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ_FILES) $(LDFLAGS)

${OBJ_DIR}:
	mkdir -p ${OBJ_DIR}

${OBJ_DIR}/%.o: ${SRC_DIR}/stages/%.cc | ${OBJ_DIR}
	${CXX} ${CXXFLAGS} -o $@ -c $<

${OBJ_DIR}/%.o: ${SRC_DIR}/%.cc | ${OBJ_DIR}
	${CXX} ${CXXFLAGS} -o $@ -c $<

all: ${TARGET}

clean:
	rm -rf ${TARGET} ${OBJ_DIR}

check: ${TARGET}
	./scripts/test_output.py -C ./test/decoder
	./scripts/test_application.sh ./test/application/testdata/level1.test
	./scripts/test_application.sh ./test/application/testdata/level2.test
	./scripts/test_application.sh ./test/application/testdata/level3.test
	./scripts/test_application.sh ./test/application/testdata/level4.test
	./scripts/test_instructions.py ./test/instruction/*.conf
	./scripts/test_instructions.py -p ./test/instruction/*.conf


