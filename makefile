# Detect platform and set OIDN path
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	OIDN_DIR = OpenImageDenoise.linux
	OIDN_RUNTIME = $(wildcard $(OIDN_DIR)/bin/*.so*)
else ifeq ($(UNAME_S),Darwin)
	OIDN_DIR = OpenImageDenoise.macos
	OIDN_RUNTIME = $(wildcard $(OIDN_DIR)/bin/*.dylib)
else
	OIDN_DIR = OpenImageDenoise.windows
	OIDN_RUNTIME = $(wildcard $(OIDN_DIR)/bin/*.dll)
endif

# Compiler and flags
CC = gcc
SRC = src/image.c src/renderer.c src/denoiser.c src/raw_render.c
SRC_PROGRAM = programs/main.c
SRC_TEST = programs/tests.c
BIN_DIR_RELEASE = bin/Release/
BIN_DIR_DEBUG = bin/Debug/
BIN_DIR_TEST = bin/Tests/
BIN_EXE = CPU_Raytracer.exe

# Ryzen 7 optimized flags
CFLAGS = -std=c17 -pthread -flto -Iinclude -I$(OIDN_DIR)/include
OPTI_FLAGS = -O3 -march=znver3 -mtune=znver3 -m64 -ffast-math -funroll-loops
LDFLAGS = -lm -lpthread -flto -L$(OIDN_DIR)/lib -lOpenImageDenoise
LDFLAGS += -Wl,-rpath,$(abspath $(OIDN_DIR)/lib)

.PHONY: build_program build_test test profile debug run clean

build_program:
	mkdir -p $(BIN_DIR_RELEASE)
	$(CC) $(CFLAGS) $(OPTI_FLAGS) $(SRC) $(SRC_PROGRAM) -o $(BIN_DIR_RELEASE)$(BIN_EXE) $(LDFLAGS)
#cp $(OIDN_RUNTIME) $(BIN_DIR_RELEASE)

build_test:
	mkdir -p $(BIN_DIR_TEST)
	$(CC) $(CFLAGS) $(OPTI_FLAGS) $(SRC) $(SRC_TEST) -o $(BIN_DIR_TEST)$(BIN_EXE) $(LDFLAGS)
#cp $(OIDN_RUNTIME) $(BIN_DIR_TEST)

debug:
	mkdir -p $(BIN_DIR_DEBUG)
	$(CC) $(CFLAGS) -g -pg $(SRC) $(SRC_PROGRAM) -o $(BIN_DIR_DEBUG)$(BIN_EXE) $(LDFLAGS)
#cp $(OIDN_RUNTIME) $(BIN_DIR_DEBUG)

test: build_test
	(cd $(BIN_DIR_TEST) && ./$(BIN_EXE))

profile: debug
	./$(BIN_DIR_DEBUG)$(BIN_EXE)
	gprof ./$(BIN_DIR_DEBUG)$(BIN_EXE) gmon.out > profile_report.txt
	@echo "Profiling results saved to profile_report.txt"
	@start notepad profile_report.txt

run: build_program
	(cd $(BIN_DIR_RELEASE) && ./$(BIN_EXE))

clean:
	rm -f $(BIN_DIR_RELEASE)$(BIN_EXE) $(BIN_DIR_DEBUG)$(BIN_EXE) $(BIN_DIR_TEST)$(BIN_EXE) profile_report.txt gmon.out