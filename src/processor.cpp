#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "../hpp/operations.hpp"
#include "../hpp/processor.hpp"
#include "../hpp/colors.hpp"

#define MEOW fprintf(stderr, "\e[0;31m" "\nmeow\n" "\e[0m");

const int HEADER_SIZE = 4;
const int REGISTER_NUM = 4;
const char* SIGNATURE = "meow";
const char* VERSION = "v.3";


typedef struct fileNames{

    const char* inputFileName;
    const char* outputFileName;
    const char* logFileName;

} fileNames_t;

typedef struct spu{
    const char*     name;

    Stack_t*        stk;
    void*           codePointer;
    void*           registersPointer;

    size_t          pc;
    size_t          sizeCommand;
    size_t          sizeArg;
    size_t          numCommands;
    size_t          memCommandsAllocated;
    size_t          numRegisters;
    size_t          memRegistersAllocated;

    size_t          errorType;                                      //errors_t?


    fileNames_t*    fileNames;
    FILE*           logFile;
    FILE*           inputFile;
    FILE*           outputFile;
} spu_t;

enum errors{
    OK_ = 0,
    ERR_NULLPTR_ = 1,
    ERR_ = 2,
    INVALID_VERSION = 3,
    INVALID_SIGNATURE = 4
};

void Run(fileNames_t* fileNames);

static errors ProcessorCtor     (spu_t* spu);
static errors ProcessorDtor     (spu_t* spu);
static errors CheckSignature    (spu_t* spu);
static errors FillCodeBuffer    (spu_t* spu);
static errors PrintFilesData    (spu_t* spu);
static errors ProcessorDump     (spu_t* spu);

/*=================================================================*/

int main(int argc, const char *argv[]){
    fileNames_t fileNames= {};

    fileNames.inputFileName  = (argc == 3) ? argv[1]  : "./bin/user_input.asm";
    fileNames.outputFileName = (argc == 3) ? argv[2]  : "stdout";
    //fileNames.logFileName    = "meow.txt";
    fileNames.outputFileName   = "meow.txt";
    Run(&fileNames);

    return 0;
}

/*=================================================================*/

static errors ProcessorCtor(spu_t* spu, const char* name){
    if (!spu) return ERR_NULLPTR_;
    if (!spu->fileNames) return ERR_NULLPTR_;

    spu->name = name;

    // WORK WITH FILES:
    const char* defaultFileNameIn  = "./bin/user_output.asm";
    const char* defaultFileNameOut = "stdout";

    spu->inputFile  = fopen(spu->fileNames->inputFileName,  "r");
    if (spu->inputFile  == nullptr)     spu->inputFile = fopen(defaultFileNameIn, "r");

    spu->outputFile = fopen(spu->fileNames->outputFileName, "w");
    if (spu->outputFile == nullptr)     spu->outputFile = stdout;

    spu->logFile    = fopen(spu->fileNames->logFileName,    "w");
    if (spu->logFile    == nullptr)     spu->logFile = stdout;

    //INITIALIZE STACK:
    spu->stk = (Stack_t*)calloc(sizeof(Stack_t), 1);
    StackCtor(spu->stk);                                             // check if allocated

    //FILL STRUCTURE FIELDS:
    spu->numRegisters   = REGISTER_NUM;
    spu->numCommands    = MAX_NUM_COMMANDS;
    spu->sizeCommand    = SIZE_COMMAND;
    spu->sizeArg        = SIZE_ARG;
    spu->pc = 0;

    //INITIALIZE CODE BUFFER:
    spu->codePointer                = calloc(spu->sizeCommand, spu->numCommands); // check if allocated
    spu->memCommandsAllocated       = spu->sizeCommand * spu->numCommands;

    //INITIALIZE REGISTER BUFFER:
    spu->registersPointer           = calloc(spu->sizeArg, spu->numRegisters);
    spu->memRegistersAllocated      = spu->sizeArg * spu->numRegisters;

    //VERIFY CODE:
    if (CheckSignature(spu)) return ERR_;

    //FILL CODE BUFFER:
    if (!spu->errorType) FillCodeBuffer(spu);
    else ProcessorDump(spu);

    return OK_;
}

/*=================================================================*/

static errors ProcessorDtor(spu_t* spu){
    if (!spu) return ERR_NULLPTR_;

    //CLOSE FILES:
    if(!spu->inputFile)                                 fclose(spu->inputFile);
    if(!spu->outputFile && spu->logFile != stdout)      fclose(spu->outputFile);
    if(!spu->logFile    && spu->logFile != stdout)      fclose(spu->logFile);

    free(spu->codePointer);
    free(spu->registersPointer);
    StackDtor(spu->stk);

    spu->memCommandsAllocated   = 0;
    spu->memRegistersAllocated  = 0;
    spu->pc                     = 0;
    spu->inputFile              = nullptr;
    spu->outputFile             = nullptr;
    spu->logFile                = nullptr;

    printf(BGRN "spu:\"%s\" destroyed\n" RESET, spu->name);

    return OK_;
}

/*=================================================================*/

static errors CheckSignature(spu_t* spu){
                                                    //verificator
    char signatureBuffer[256] = "";
    int version = -1;

    for (int i = 0; i < HEADER_SIZE; i++){

        fscanf(spu->inputFile, "%s", signatureBuffer);

        if (!strcmp(signatureBuffer, "signature:")){

            fscanf(spu->inputFile, "%s", signatureBuffer);

            fprintf(stderr, "\n[%s]OK\n", signatureBuffer);

            if (strcmp(signatureBuffer, SIGNATURE)){
                printf(RED "invalid signature\n" RESET);
                spu->errorType = INVALID_SIGNATURE;

                return INVALID_SIGNATURE;
            }
        }

        else if (!strcmp(signatureBuffer, "version:")){

            fscanf(spu->inputFile, "%s", signatureBuffer);

            fprintf(stderr, "\n[%s]OK\n", signatureBuffer);

            if (strcmp(signatureBuffer, VERSION)){
                printf(RED "invalid version\n" RESET);
                spu->errorType = INVALID_VERSION;

                return INVALID_VERSION;
            }
        }
    }

    return OK_;
}

/*=================================================================*/

static errors FillCodeBuffer(spu_t* spu){
                                                        //verificator
    for (int ip = 0; ip < MAX_NUM_COMMANDS; ip++){
        int64_t* pointer = (int64_t*)spu->codePointer + ip;
        fscanf(spu->inputFile, "%lld\n", pointer);
    }

    return OK_;
}

/*=================================================================*/

static errors PrintFilesData(spu_t* spu){
    if (!spu) return ERR_NULLPTR_;
    FILE* logFile = spu->logFile;

    if (logFile == stdout) printf(MAG);
    fprintf(logFile, "\n");
    fprintf(logFile, "input file pointer: \t%p\n",  spu->inputFile);
    fprintf(logFile, "input file name:    \t%s\n",  spu->fileNames->inputFileName);

    fprintf(logFile, "output file pointer:\t%p\n", spu->outputFile);
    fprintf(logFile, "output file name:   \t%s\n", spu->fileNames->outputFileName);

    fprintf(logFile, "log file pointer:   \t%p\n", spu->logFile);
    fprintf(logFile, "log file name:      \t%s\n", spu->fileNames->logFileName);
    fprintf(logFile, "\n");
    if (logFile == stdout) printf(RESET);

    return OK_;
}

/*=================================================================*/

static errors ProcessorDump(spu_t* spu){
    if (!spu->logFile){
        spu->logFile = stdout;
        FILE* outputFile = spu->logFile;
    }

    FILE* logFile = spu->logFile;

    fprintf(logFile, "=================================================\n");

    if (logFile == stdout) printf(GRN);
    fprintf(logFile, "dump of \"%s\":\n", spu->name);
    if (logFile == stdout) printf(RESET);

    PrintFilesData(spu);

    //ERRORS:
    if (logFile == stdout) printf(YEL);
    switch (spu->errorType){

        case INVALID_VERSION:{
            fprintf(logFile, "\nError: %lu - invalid version\n\n",  spu->errorType);
            break;
        }

        case INVALID_SIGNATURE:{
            fprintf(logFile, "\nError: %lu - invalid signature\n\n",  spu->errorType);
            break;
        }

        default:{
            fprintf(logFile, "\nError: %lu\n\n",  spu->errorType);
            break;
        }
    }
    if (logFile == stdout) printf(RESET);

    //STRUCT POLES:
    if (logFile == stdout) printf(CYN);
    fprintf(logFile, "Struct pointer:                       \t%p\n",  spu);
    fprintf(logFile, "Code pointer:                         \t%p\n",  spu->codePointer);
    fprintf(logFile, "\n");
    fprintf(logFile, "Number of commands:                   \t%lu\n", spu->numCommands);
    fprintf(logFile, "Size of command:                      \t%lu\n", spu->sizeCommand);
    fprintf(logFile, "Allocated memory for commands(bytes): \t%lu\n", spu->memCommandsAllocated);
    if (logFile == stdout) printf(RESET);


    //CODE BUFFER:
    if (!spu->codePointer){

        if (logFile == stdout) printf(BRED);
        fprintf(logFile, "spu code does not exist\n");
        if (logFile == stdout) printf(RESET);

    }

    else{
        uint64_t* cmdPtr = (uint64_t*)(spu->codePointer);                           // change type

        if (logFile == stdout) printf(GRN);
        fprintf(logFile, "Commands:\n");
        if (logFile == stdout) printf(RESET);

        for (size_t ip = 0; ip < MAX_NUM_COMMANDS; ip++){

            fprintf(spu->logFile, "pc<%lu>:\t%lld", ip, *(cmdPtr + ip));

            if (ip == spu->pc){
                if (spu->logFile == stdout) fprintf(spu->logFile, BRED  "\t\t <---- ded" RESET);
                else                        fprintf(spu->logFile,       "\t\t <---- ded");
            }

            fprintf(spu->logFile, "\n");
        }
    }

    fprintf(logFile, "---------------------------\n");

    //REGISTERS:
    if (logFile == stdout) printf(CYN);
    fprintf(logFile, "Number of registers:                  \t%lu\n", spu->numRegisters);
    fprintf(logFile, "Size of argument:                     \t%lu\n", spu->sizeArg);
    fprintf(logFile, "Allocated memory for registers(bytes):\t%lu\n", spu->memRegistersAllocated);
    if (logFile == stdout) printf(RESET);

    fprintf(logFile, "\n");

    //REGISTERS BUFFER:
    if (!spu->registersPointer){
        if (logFile == stdout) printf(BRED);
        fprintf(logFile, "spu registers do not exist\n");
        if (logFile == stdout) printf(RESET);
    }

    else{
        uint64_t* regPtr = (uint64_t*)(spu->registersPointer);

        if (logFile == stdout) printf(GRN);
        fprintf(logFile, "Registers:\n");
        if (logFile == stdout) printf(RESET);

        for (int numReg = 0; numReg < spu->numRegisters; numReg++){
            fprintf(spu->logFile, "r<%d>:\t%lld\n", numReg, *(regPtr + numReg));
        }
    }


    fprintf(logFile, "=================================================\n");

    //WAIT USER INPUT
    if (logFile == stdout) getchar();

    return OK_;
}

/*=================================================================*/

void Run(fileNames_t* fileNames){

    spu_t spu = {};
    spu.fileNames = fileNames;

    bool RunCommands = 1;

    if (ProcessorCtor(&spu, "1")){
        ProcessorDump(&spu);
        RunCommands = 0;
    }

    ProcessorDump(&spu);

    while (RunCommands){
        if (spu.pc > MAX_NUM_COMMANDS){
            RunCommands = 0;
            ProcessorDump(&spu);
        }

        char* nextArg = (char*)spu.codePointer + spu.pc * spu.sizeArg;
        printf("0x%x\n", *nextArg);

        const char OPERATOR_MUSK = 0x0F;
        switch (*nextArg & OPERATOR_MUSK){

            case PUSH:{
                char pushVariant = *nextArg & 0xF0;

                switch(pushVariant){
                    case 0x10:{
                        int64_t num_in = 0;
                        num_in = *(nextArg + 1 * spu.sizeCommand);

                        StackPush(spu.stk, num_in);

                        spu.pc += 2;
                        break;
                    }

                    case 0x20:{
                        int64_t num_in = 0, arg_out = 0;
                        num_in  = *(nextArg + 1 * spu.sizeCommand);
                        arg_out = *((int64_t*)spu.registersPointer + num_in - 1);

                        StackPush(spu.stk, arg_out);

                        spu.pc += 2;
                        break;
                    }

                    case 0x30:{
                        int64_t frst_num_in = 0, arg_out = 0, scnd_num_in = 0, num_reg = 0;

                        frst_num_in = *(nextArg + 1 * spu.sizeArg);
                        num_reg     = *((int64_t*)spu.registersPointer + frst_num_in - 1);

                        scnd_num_in = *(nextArg + 2 * spu.sizeArg);

                        arg_out = num_reg + scnd_num_in;
                        StackPush(spu.stk, arg_out);

                        spu.pc += 3;
                        break;
                    }

                    default:{

                        break;
                    }
                }

                break;
            }

            case POP:{
                int64_t first_arg = 0, frst_num_in = 0, num_reg = 0;

                frst_num_in = *(nextArg + 1 * spu.sizeCommand);

                StackPop(spu.stk, &first_arg);

                *((int64_t*)spu.registersPointer + frst_num_in - 1) = first_arg;

                spu.pc += 2;
                break;
            }

            case ADD:{
                int64_t num_first = 0, num_second = 0;

                StackPop(spu.stk, &num_first);
                StackPop(spu.stk, &num_second);

                StackPush(spu.stk, num_first + num_second);

                spu.pc++;
                break;
            }

            case SUB:{
                int64_t positive = 0, negative = 0;

                StackPop(spu.stk, &positive);
                StackPop(spu.stk, &negative);

                StackPush(spu.stk, positive - negative);

                spu.pc++;
                break;
            }

            case MUL:{
                int64_t num_first = 0, num_second = 0;

                StackPop(spu.stk, &num_first);
                StackPop(spu.stk, &num_second);

                StackPush(spu.stk, num_first * num_second);

                spu.pc++;
                break;
            }

            case DIV:{
                int64_t numerator = 0, divisor = 0;

                StackPop(spu.stk, &numerator);
                StackPop(spu.stk, &divisor);

                StackPush(spu.stk, numerator / divisor);

                spu.pc++;
                break;
            }

            case SQRT:{
                int64_t num = 0;

                StackPop(spu.stk, &num);

                num = (num >= 0) ? sqrt(num) : 0;

                StackPush(spu.stk, num);

                spu.pc++;
                break;
            }

            case SIN:{
                int64_t num = 0;

                StackPop(spu.stk, &num);

                num = sin(num);

                StackPush(spu.stk, num);

                spu.pc++;
                break;
            }

            case COS:{
                int64_t num = 0;

                StackPop(spu.stk, &num);

                num = cos(num);

                StackPush(spu.stk, num);

                spu.pc++;
                break;
            }

            case OUT:{
                int64_t num_out = 0;

                StackPop(spu.stk, &num_out);

                fprintf(spu.outputFile, "%lld\n", num_out);

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

            case DUMP:{
                ProcessorDump(&spu);
                StackDump(spu.stk);

                spu.pc++;
                break;
            }

            case HLT:{
                RunCommands = 0;

                spu.pc++;
                break;
            }

            default :{
                printf(RED "\nERROR\n" RESET);

                spu.pc++;
                break;
            }
        }
    }

    ProcessorDtor(&spu);
}

