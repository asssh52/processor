#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../hpp/colors.hpp"
#include "../hpp/operations.hpp"

void Compile(const char* inputFileName);

int main(int argc, const char* argv[]){
    const char* inputFile = (argc == 2) ? argv[1] : "./bin/user_input.txt";

    Compile(inputFile);

    return 0;
}




void Compile(const char* inputFileName){

    char* defaultFileNameIn  = "./bin/user_input.txt";
    char* defaultFileNameOut = "./bin/user_input.asm";

    FILE* inputFile = fopen(inputFileName, "r");
    if (inputFile == nullptr){
        inputFile = fopen(defaultFileNameIn, "r");
    }

    FILE* outputFile = fopen(defaultFileNameOut, "w");

    bool RunCommands = 1;
    while (RunCommands){
        char cmd[99] = "";
        fscanf(inputFile, "%s", cmd);
        if (!strcmp(cmd, "push")){
            uint64_t arg = 0;
            fscanf(inputFile, "%lld", &arg);

            fprintf(outputFile, "%d %lld\n", PUSH, arg);
        }

        else if (!strcmp(cmd, "sub")){
            fprintf(outputFile, "%d\n", SUB);
        }

        else if (!strcmp(cmd, "out")){
            fprintf(outputFile, "%d\n", OUT);
        }

        else if (!strcmp(cmd, "hlt")){
            fprintf(outputFile, "%d\n", HLT);
            RunCommands = 0;
        }

        else{
            fprintf(outputFile, "ERROR\n");
        }
    }

}

