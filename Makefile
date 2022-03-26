CPP             = g++
RM              = rm -f

PACKAGE		= bfbj
BUILD_DIR	= ./build
LOCALE_DIR	= ./locale
ROOT 		= ./src
OPENSSL_DIR		= ${ROOT}/openssl
LIBRARY_DIRS	= -L$(OPENSSL_DIR)/lib -L${ROOT} -L${ROOT}/fmt/ -lssl -lz -lcrypto -lfmt -lspdlog -lpthread -ldl ${ROOT}/sqlite/sqlite3.o
HEADERS			= -I$(OPENSSL_DIR)/include

CPP_COMMON	= -Wall -c -I${ROOT} -std=c++14 -fpic
debug:		CPP_FLAGS       = $(CPP_COMMON) -Wextra -O0 -DDEBUG -g
release:	CPP_FLAGS       = $(CPP_COMMON) -O2
MKDIR_P			= mkdir -p


LD              = g++
debug: LD_FLAGS        = -Wall -Wextra -O0
release: LD_FLAGS        = -Wall -O2
EXE				= bot

SOURCES		= $(wildcard ${ROOT}/sqlite/*.cpp) $(wildcard ${ROOT}/pugixml/*.cpp) $(wildcard ${ROOT}/channel/*.cpp) $(wildcard ${ROOT}/strategy/*.cpp) $(wildcard ${ROOT}/*.cpp)
OBJECTS		= $(SOURCES:%=$(BUILD_DIR)/%.o)

debug:		clear clean_exe ${OBJECTS} ${EXE}
release:	clear clean_exe ${OBJECTS} ${EXE}

all:	${OBJECTS} ${EXE}

${EXE}: ${OBJECTS}
	${LD} -o ./$@ ${OBJECTS} ${LIBRARY_DIRS} ${LD_FLAGS}

$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	${CPP} ${CPP_FLAGS} -c $< -o $@ $(HEADERS)

clean_dev:
	${RM} ${OBJECTS}

clean_exe:
	${RM} ./${EXE}

.PHONY: clean

clean:	clean_dev clean_exe

clear:
	clear

run:
	./${EXE}

