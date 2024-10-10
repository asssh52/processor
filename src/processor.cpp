#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
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

                break;
            }

            case ADD :{
                uint64_t num_first = 0, num_second = 0;

                StackPop(&stk, &num_first);
                StackPop(&stk, &num_second);

                StackPush(&stk, num_first + num_second);

                break;
            }

            case SUB :{
                printf(MAG "\n%d\n3\n" RESET, cmd);
                uint64_t positive = 0, negative = 0;

                StackPop(&stk, &positive);
                StackPop(&stk, &negative);

                StackPush(&stk, positive - negative);

                break;
            }

            case MUL :{
                uint64_t num_first = 0, num_second = 0;

                StackPop(&stk, &num_first);
                StackPop(&stk, &num_second);

                StackPush(&stk, num_first * num_second);

                break;
            }

            case DIV :{
                uint64_t numerator = 0, divisor = 0;

                StackPop(&stk, &numerator);
                StackPop(&stk, &divisor);

                StackPush(&stk, numerator / divisor);

                break;
            }

            case SQRT :{
                uint64_t num = 0;

                StackPop(&stk, &num);

                num = (num >= 0) ? sqrt(num) : 0;

                StackPush(&stk, num);

                break;
            }

            case SIN :{
                uint64_t num = 0;

                StackPop(&stk, &num);

                num = sin(num);

                StackPush(&stk, num);

                break;
            }

            case COS :{
                uint64_t num = 0;

                StackPop(&stk, &num);

                num = cos(num);

                StackPush(&stk, num);

                break;
            }

            case OUT :{
                uint64_t num_out = 0;

                StackPop(&stk, &num_out);

                printf(MAG "%ld\n" RESET, num_out);

                break;
            }

            case IN :{
                uint64_t num_in = 0;

                printf(CYN "enter num:\n" RESET);
                scanf("%lld", &num_in);

                StackPush(&stk, num_in);

                break;
            }

            case DUMP :{
                StackDump(&stk);

                break;
            }

            case HLT :{
                RunCommands = 0;

                break;
            }

            default  :{
                printf(RED "\nERROR\n" RESET);

                break;
            }
        }
    }
}

