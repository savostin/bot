CPP             = g++
RM              = rm -f
debug:		CPP_FLAGS       = -Wall -Wextra -c -I. -O0 -std=c++14 -fpic -DDEBUG -g
release:	CPP_FLAGS       = -Wall -c -I. -O2 -std=c++14 -fpic
MKDIR_P			= mkdir -p

OPENSSL_DIR		= /usr/local/opt/openssl
LIBRARY_DIRS	= -L$(OPENSSL_DIR)/lib -L./spdlog/ -L. -lssl -lz -lcrypto -lfmt -lspdlog
HEADERS			= -I$(OPENSSL_DIR)/include -I./spdlog/

LD              = g++
debug: LD_FLAGS        = -Wall -Wextra -O0
release: LD_FLAGS        = -Wall -O2
EXE				= bot
say				= say "Done"

BUILD_DIR		= ./build
SOURCES			= $(wildcard pugixml/*.cpp) $(wildcard channel/*.cpp) $(wildcard strategy/*.cpp) $(wildcard *.cpp)
#SOURCES			= main.cpp 
OBJECTS         = $(SOURCES:%=$(BUILD_DIR)/%.o)

debug:	clear clean_exe ${OBJECTS} ${EXE} say
release:	clear clean_exe ${OBJECTS} ${EXE}

all:	${OBJECTS} ${EXE}

${EXE}: ${OBJECTS}
	${LD} -o ./$@ ${OBJECTS} ${LIBRARY_DIRS} ${LD_FLAGS}

#${OBJECTS}: ${SOURCES}
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

say:
	${SAY}