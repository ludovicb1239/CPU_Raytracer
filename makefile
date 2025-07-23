# Compiler and flags
CC = gcc
SRC = main.c 01_09_02_bmp.c
BIN_DIR_RELEASE = bin/Release/
BIN_DIR_DEBUG = bin/Debug/
BIN_EXE = CPU_Raytracer.exe

# Ryzen 7 optimized flags
CFLAGS = -std=c17 -pthread -flto
OPTI_FLAGS = -O3 -march=znver3 -mtune=znver3 -m64 -ffast-math -funroll-loops
LDFLAGS = -lm -lpthread -flto -IOpenImageDenoise/include -L OpenImageDenoise/lib -lOpenImageDenoise

.PHONY: build debug run clean

build:
	$(CC) $(CFLAGS) $(OPTI_FLAGS) $(SRC) -o $(BIN_DIR_RELEASE)$(BIN_EXE) $(LDFLAGS)

debug:
	$(CC) $(CFLAGS) -g -pg $(SRC) -o $(BIN_DIR_DEBUG)$(BIN_EXE) $(LDFLAGS)

profile: debug
	./$(BIN_DEBUG)
	gprof ./$(BIN_DEBUG) gmon.out > profile_report.txt
	@echo "Profiling results saved to profile_report.txt"
	@start notepad profile_report.txt

run: build
	(cd $(BIN_DIR_RELEASE) && ./$(BIN_EXE))

clean:
	rm -f $(BIN_DIR_RELEASE) $(BIN_DIR_DEBUG) profile_report.txt gmon.out
