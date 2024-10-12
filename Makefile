CXX_FLAGS = -D _DEBUG -ggdb3 -std=c++17 -O0 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations \
   -Wc++14-compat -Wmissing-declarations -Wcast-qual -Wchar-subscripts                             \
   -Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal           \
   -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline                   \
   -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked                     \
   -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo           \
   -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn                         \
   -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default               \
   -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast                    \
   -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing            \
   -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation             \
   -fstack-protector -fstrict-overflow -fno-omit-frame-pointer -Wlarger-than=8192                  \
   -Wstack-usage=8192 -fsanitize=address -fsanitize=undefined -fPIE -Werror=vla -Wno-format

CXX = clang++

all: run

compile: ./bin/compiler.o
	$(CXX) ./bin/compiler.o $(CXXFLAGS) -o compile

./bin/compiler.o: ./src/compiler.cpp
	$(CXX) -c ./src/compiler.cpp $(CXXFLAGS) -o ./bin/compiler.o

run:       ./bin/processor.o ./mystack/mystack.o
	$(CXX) ./bin/processor.o     ./bin/mystack.o $(CXXFLAGS) -o main

./mystack/mystack.o: ../mystack/mystack.cpp
	$(CXX) -c        ../mystack/mystack.cpp $(CXXFLAGS) -o ./bin/mystack.o

./bin/processor.o:        src/processor.cpp hpp/processor.hpp
	$(CXX) -c           ./src/processor.cpp $(CXXFLAGS) -o ./bin/processor.o

clean:
	rm -f main compile ./bin/*.o
