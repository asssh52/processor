#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "../hpp/operations.hpp"
#include "../hpp/processor.hpp"
#include "../hpp/colors.hpp"

#define MEOW fprintf(stderr, "\e[0;31m" "\nmeow\n" "\e[0m");

const int HEADER_SIZE = 4;
const char* SIGNATURE = "meow";
const int VERSION = 1;

typedef struct fileNames{

    const char* inputFileName;
    const char* outputFileName;
    const char* logFileName;

} fileNames_t;

typedef struct spu{
    const char*     name;

    Stack_t*        stk;
    void*           codePointer;
    size_t          numCommands;
    size_t          pc;
    size_t          sizeCommand;
    size_t          sizeAllocated;

    fileNames_t*    fileNames;
    FILE*           logFile;
    FILE*           inputFile;
    FILE*           outputFile;
} spu_t;

enum errors{
    OK_P = 0,
    ERR_NULLPTR_P = 1
};

void Run(fileNames_t* fileNames);

static errors ProcessorCtor     (spu_t* spu);
static errors ProcessorDtor     (spu_t* spu);
static errors CheckSignature    (spu_t* spu);
static errors FillCodeBuffer    (spu_t* spu);

/*=================================================================*/

int main(int argc, const char *argv[]){
    fileNames_t fileNames= {};

    fileNames.inputFileName  = (argc == 3) ? argv[1]  : "./bin/user_input.asm";
    fileNames.outputFileName = (argc == 3) ? argv[2]  : "stdout";

    Run(&fileNames);

    return 0;
}

/*=================================================================*/

static errors ProcessorCtor(spu_t* spu){
    if (!spu) return ERR_NULLPTR_P;
    if (!spu->fileNames) return ERR_NULLPTR_P;

    // WORKING WITH FILES:
    const char* defaultFileNameIn  = "./bin/user_output.asm";
    const char* defaultFileNameOut = "stdout";

    FILE* inputFile  = fopen(spu->fileNames->inputFileName, "r");
    if (inputFile == nullptr)  spu->inputFile = fopen(defaultFileNameIn, "r");

    FILE* outputFile = fopen(spu->fileNames->outputFileName, "w");
    if (outputFile == nullptr) spu->outputFile = stdout;

    FILE* logFile    = fopen(spu->fileNames->logFileName, "w");
    if (logFile == nullptr) spu->logFile = stdout;


    //INITIALIZING STACK:
    spu->stk = (Stack_t*)calloc(sizeof(Stack_t), 1);
    StackCtor(spu->stk);                                             // check if allocated
    fprintf(stderr, "\nmeow\n");
    //FILLING STRUCTURE FIELDS:
    spu->numCommands = MAX_NUM_COMMANDS;
    spu->sizeCommand = SIZE_COMMAND;

    spu->pc = 0;
    fprintf(stderr, "\nmeow\n");
    //INITIALIZING CODE BUFFER:
    spu->codePointer   = calloc(spu->sizeCommand, spu->numCommands); // check if allocated
    spu->sizeAllocated = spu->sizeCommand * spu->numCommands;
    fprintf(stderr, "\nmeow\n");
    //VERIFY CODE:
    CheckSignature(spu);
    fprintf(stderr, "\nmeow\n");
    //FILL CODE BUFFER:
    FillCodeBuffer(spu);

    return OK_P;
}

/*=================================================================*/

static errors ProcessorDtor(spu_t* spu){
    if (!spu) return ERR_NULLPTR_P;

    //CLOSING FILES:
    if(!spu->inputFile)     fclose(spu->inputFile);
    if(!spu->outputFile)    fclose(spu->outputFile);
    if(!spu->logFile)       fclose(spu->logFile);

    free(spu->codePointer);
    StackDtor(spu->stk);

    spu->sizeAllocated  = 0;
    spu->pc             = 0;
    spu->inputFile      = nullptr;
    spu->outputFile     = nullptr;
    spu->logFile        = nullptr;

    printf("spu:%s destroyed", spu->name);

    return OK_P;
}

/*=================================================================*/

static errors CheckSignature(spu_t* spu){
                                                    //verificator
    char signatureBuffer[256] = "";
    int version = 0;

    MEOW
    fscanf(spu->inputFile, "signature:%s\n", signatureBuffer);
    fscanf(spu->inputFile, "version:%d", &version);
    MEOW

    fprintf(stderr, "\n[%s]\n", signatureBuffer);
    fprintf(stderr, "\n[%d]\n", version);
    /*
    if (version != VERSION){
        printf("VERSION\n");
        abort();
    }*/

    return OK_P;
}

/*=================================================================*/

static errors FillCodeBuffer(spu_t* spu){
                                                        //verificator

    /*for (int i = 0; i < HEADER_SIZE; i++){
        fscanf(spu->inputFile, "\n");
    }*/

    for (int ip = 0; ip < MAX_NUM_COMMANDS; ip++){

        int64_t* pointer = (int64_t*)spu->codePointer + ip;

        int a = fscanf(spu->inputFile, "%d\n", pointer);
        printf("a:%d\n", a);

        printf("%ld\n", *pointer);

        if (*pointer == 1) ip++;
    }

    return OK_P;
}

/*=================================================================*/

void Run(fileNames_t* fileNames){

    spu_t spu = {};
    spu.fileNames = fileNames;

    ProcessorCtor(&spu);
    fprintf(stderr, "\nmeow\n");


    bool RunCommands = 1;
    while (RunCommands){
        if (spu.pc > 10) break;
        int64_t* nextArg = (int64_t*)spu.codePointer + spu.pc;

        switch (*nextArg){

            case PUSH:{
                int64_t num_in = 0;
                num_in = *(nextArg + 1);

                StackPush(spu.stk, num_in);

                spu.pc += 2;
                break;
            }

            case ADD :{
                int64_t num_first = 0, num_second = 0;

                StackPop(spu.stk, &num_first);
                StackPop(spu.stk, &num_second);

                StackPush(spu.stk, num_first + num_second);

                spu.pc++;
                break;
            }

            case SUB :{
                int64_t positive = 0, negative = 0;

                StackPop(spu.stk, &positive);
                StackPop(spu.stk, &negative);

                StackPush(spu.stk, positive - negative);

                spu.pc++;
                break;
            }

            case MUL :{
                int64_t num_first = 0, num_second = 0;

                StackPop(spu.stk, &num_first);
                StackPop(spu.stk, &num_second);

                StackPush(spu.stk, num_first * num_second);

                spu.pc++;
                break;
            }

            case DIV :{
                int64_t numerator = 0, divisor = 0;

                StackPop(spu.stk, &numerator);
                StackPop(spu.stk, &divisor);

                StackPush(spu.stk, numerator / divisor);

                spu.pc++;
                break;
            }

            case SQRT :{
                int64_t num = 0;

                StackPop(spu.stk, &num);

                num = (num >= 0) ? sqrt(num) : 0;

                StackPush(spu.stk, num);

                spu.pc++;
                break;
            }

            case SIN :{
                int64_t num = 0;

                StackPop(spu.stk, &num);

                num = sin(num);

                StackPush(spu.stk, num);

                spu.pc++;
                break;
            }

            case COS :{
                int64_t num = 0;

                StackPop(spu.stk, &num);

                num = cos(num);

                StackPush(spu.stk, num);

                spu.pc++;
                break;
            }

            case OUT :{
                int64_t num_out = 0;

                StackPop(spu.stk, &num_out);

                printf(MAG "%lld\n" RESET, num_out);

                spu.pc++;
                break;
            }

            case IN :{
                int64_t num_in = 0;

                printf(CYN "enter num:\n" RESET);
                scanf("%lld", &num_in);

                StackPush(spu.stk, num_in);

                spu.pc++;
                break;
            }

            case DUMP :{
                StackDump(spu.stk);

                spu.pc++;
                break;
            }

            case HLT :{
                RunCommands = 0;

                spu.pc++;
                break;
            }

            default  :{
                printf(RED "\nERROR\n" RESET);

                spu.pc++;
                break;
            }
        }
    }

    ProcessorDtor(&spu);
}

