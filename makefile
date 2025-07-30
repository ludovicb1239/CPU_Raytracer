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
CFLAGS = -std=c17 -pthread -flto
OPTI_FLAGS = -O3 -march=znver3 -mtune=znver3 -m64 -ffast-math -funroll-loops
LDFLAGS = -Iinclude -lm -lpthread -flto -IOpenImageDenoise/include -L OpenImageDenoise/lib
#-lOpenImageDenoise

.PHONY: build_program build_test test profile debug run clean

build_program:
	$(CC) $(CFLAGS) $(OPTI_FLAGS) $(SRC) $(SRC_PROGRAM) -o $(BIN_DIR_RELEASE)$(BIN_EXE) $(LDFLAGS)

build_test:
	$(CC) $(CFLAGS) $(OPTI_FLAGS) $(SRC) $(SRC_TEST) -o $(BIN_DIR_TEST)$(BIN_EXE) $(LDFLAGS)

debug:
	$(CC) $(CFLAGS) -g -pg $(SRC) $(SRC_PROGRAM) -o $(BIN_DIR_DEBUG)$(BIN_EXE) $(LDFLAGS)

test: build_test
	(cd $(BIN_DIR_TEST) && ./$(BIN_EXE))


profile: debug
	./$(BIN_DEBUG)
	gprof ./$(BIN_DEBUG) gmon.out > profile_report.txt
	@echo "Profiling results saved to profile_report.txt"
	@start notepad profile_report.txt

run: build_program
	(cd $(BIN_DIR_RELEASE) && ./$(BIN_EXE))

clean:
	rm -f $(BIN_DIR_RELEASE) $(BIN_DIR_DEBUG) profile_report.txt gmon.out
