#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../hpp/operations.hpp"
#include "../hpp/processor.hpp"
#include "../hpp/colors.hpp"

void Run(const char* inputFileName){

    char* defaultFileName = "./bin/user_input.asm";

    FILE* inputFile = fopen(inputFileName, "r");
    if (!inputFile){
        inputFile = fopen(defaultFileName, "r");
    }

    fprintf(stderr, "\n%p\n\n", inputFile);
    fprintf(stderr, "\n%s\n\n", defaultFileName);

    Stack_t stk = {};
    StackCtor(&stk);

    bool RunCommands = 1;
    while (RunCommands){

        int cmd = HLT;
        fscanf(inputFile, "%d", &cmd);

        switch (cmd){

            case PUSH:{
                uint64_t num_in = 0;
                fscanf(inputFile, "%lld", &num_in);

                StackPush(&stk, num_in);
            }

            case SUB :{
                uint64_t positive = 0, negative = 0;

                StackPop(&stk, &negative);
                StackPop(&stk, &positive);

                StackPush(&stk, positive - negative);
            }

            case OUT :{
                uint64_t num_out = 0;

                StackPop(&stk, &num_out);

                printf(MAG "%ld\n" RESET, num_out);
            }

            case HLT :{
                RunCommands = 0;
            }

            default  :{
                printf(RED "\nERROR\n" RESET);
            }
        }
    }
}

