#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../hpp/colors.hpp"
#include "../hpp/compiler.hpp"
#include "../hpp/operations.hpp"

const char* SIGNATURE = "meow";
const int VERSION = 1;

static void Compile(fileNames_t* fileNames);

int main(int argc, const char* argv[]){
    fileNames_t fileNames= {};

    fileNames.inputFileName  = (argc == 3) ? argv[1]  : "./bin/user_input.txt";
    fileNames.outputFileName = (argc == 3) ? argv[2]  : "./bin/user_output.asm";

    Compile(&fileNames);

    return 0;
}

/*=======================================================================*/

static errors PrintFilesData(commands* codeStruct){
    if (!codeStruct) return ERR_NULLPTR;
    FILE* logFile = codeStruct->logFile;
    logFile = stdout;

    if (logFile == stdout) printf(MAG);
    fprintf(logFile, "\n");
    fprintf(logFile, "input file pointer:\t%p\n",  codeStruct->inputFile);
    fprintf(logFile, "input file name:   \t%s\n",  codeStruct->fileNames->inputFileName);

    fprintf(logFile, "output file pointer:\t%p\n", codeStruct->outputFile);
    fprintf(logFile, "output file name:   \t%s\n", codeStruct->fileNames->outputFileName);

    fprintf(logFile, "log file pointer:   \t%p\n", codeStruct->logFile);
    fprintf(logFile, "log file name:      \t%s\n", codeStruct->fileNames->logFileName);
    fprintf(logFile, "\n");
    if (logFile == stdout) printf(RESET);

    return OK;
}


/*=======================================================================*/

static errors CommandsCtor(commands* codeStruct, const char* name){
    if (!codeStruct) return ERR_NULLPTR;

    const char* defaultFileNameIn  = "./bin/user_input.txt";
    const char* defaultFileNameOut = "./bin/user_output.asm";

    FILE* inputFile  = fopen(codeStruct->fileNames->inputFileName, "r");
    if (inputFile == nullptr)  inputFile = fopen(defaultFileNameIn, "r");

    FILE* outputFile = fopen(codeStruct->fileNames->outputFileName, "w");
    if (outputFile == nullptr) outputFile = fopen(defaultFileNameOut, "w");

    FILE* logFile = fopen(codeStruct->fileNames->logFileName, "w");
    if (logFile == nullptr) logFile = stdout;

    codeStruct->inputFile  = inputFile;
    codeStruct->outputFile = outputFile;
    codeStruct->logFile    = logFile;

    codeStruct->numCommands = MAX_NUM_COMMANDS;                                         //count commands
    codeStruct->sizeArg = SIZE_COMMAND;

    codeStruct->name = name;
    codeStruct->codePointer = calloc(codeStruct->sizeArg, codeStruct->numCommands);     //exception nlptr = calloc
    codeStruct->sizeAllocated = codeStruct->numCommands * codeStruct->sizeArg;

    return OK;
}

/*=======================================================================*/

static errors CommandsDtor(commands* codeStruct){
    if (!codeStruct->codePointer) return ERR_NULLPTR;

    free(codeStruct->codePointer);

    if (!codeStruct->logFile && codeStruct->logFile != stdout) fclose(codeStruct->logFile);
    if (!codeStruct->inputFile)                                fclose(codeStruct->inputFile);
    if (!codeStruct->outputFile)                               fclose(codeStruct->outputFile);

    fprintf(codeStruct->logFile, CYN "%s destoyed\n"  RESET, codeStruct->name);

    return OK;
}

/*=======================================================================*/

static errors CommandsDump(commands* codeStruct){
    if (!codeStruct->logFile) codeStruct->logFile = stdout;
        FILE* outputFile = codeStruct->logFile;
    FILE* logFile = codeStruct->logFile;

    fprintf(logFile, "=================================================\n");

    if (logFile == stdout) printf(GRN);
    fprintf(logFile, "dump of \"%s\":\n", codeStruct->name);
    if (logFile == stdout) printf(RESET);

    PrintFilesData(codeStruct);

    if (logFile == stdout) printf(CYN);
    fprintf(logFile, "Struct pointer:                       \t%p\n",  codeStruct);
    fprintf(logFile, "Code pointer:                         \t%p\n",  codeStruct->codePointer);
    fprintf(logFile, "Number of commands:                   \t%lu\n", codeStruct->numCommands);
    fprintf(logFile, "Size of command:                      \t%lu\n", codeStruct->sizeArg);
    fprintf(logFile, "Allocated memory for commands(bytes): \t%lu\n", codeStruct->sizeAllocated);
    if (logFile == stdout) printf(RESET);

    if (!codeStruct->codePointer){
        if (logFile == stdout) printf(RED);
        fprintf(logFile, "codeStruct does not exist\n");
        if (logFile == stdout) printf(RESET);
    }

    uint64_t* cmdPtr = (uint64_t*)(codeStruct->codePointer);                           // change type

    if (logFile == stdout) printf(GRN);
    fprintf(logFile, "Commands:\n");
    if (logFile == stdout) printf(RESET);

    for (size_t pc = 0; pc < MAX_NUM_COMMANDS; pc++){
        if (*((uint64_t*)(codeStruct->codePointer)+ pc) == PUSH){
            fprintf(codeStruct->logFile, "pc<%lu>:\t%lld %lld\n", pc, *(cmdPtr+ pc), *(cmdPtr+ pc + 1));
            pc++;
        }

        else{
            fprintf(codeStruct->logFile, "pc<%lu>:\t%lld\n", pc, *(cmdPtr + pc));
        }
    }
    fprintf(logFile, "=================================================\n");

    return OK;
}

/*=======================================================================*/

static errors PrintSignature(commands* codeStruct){
    FILE* outputFile = codeStruct->outputFile;

    fprintf(outputFile, "=======================================\n");
    fprintf(outputFile, "signature: %s\n", SIGNATURE);
    fprintf(outputFile, "version: v.%d\n", VERSION);
    fprintf(outputFile, "=======================================\n");

    return OK;
}

/*=======================================================================*/

static errors OutputCode(commands* codeStruct){
    if (!codeStruct || !codeStruct->codePointer) return ERR_NULLPTR;
                                                                                        //add file verifycator

    uint64_t* cmdPtr = (uint64_t*)(codeStruct->codePointer);                            // change type

    for (size_t pc = 0; pc < MAX_NUM_COMMANDS; pc++){
        if (*((uint64_t*)(codeStruct->codePointer)+ pc) == PUSH){
            fprintf(codeStruct->outputFile, "%lld %lld\n", *(cmdPtr+ pc), *(cmdPtr+ pc + 1));
        }

        else{
            fprintf(codeStruct->outputFile, "%lld\n", *(cmdPtr + pc));
        }
    }

    return OK;
}

/*=======================================================================*/

static void Compile(fileNames_t* fileNames){
    commands codeStruct = {};
    codeStruct.fileNames = fileNames;

    CommandsCtor(&codeStruct, "mycode");

    CommandsDump(&codeStruct);

    PrintSignature(&codeStruct);

    FILE* inputFile  = codeStruct.inputFile;
    FILE* outputFile = codeStruct.outputFile;

    bool RunCommands = 1;
    size_t pc = 0;

    while (RunCommands){
        char cmd[99] = "";
        fscanf(inputFile, "%s", cmd);

        if (!strcmp(cmd, "push")){
            uint64_t arg = 0;
            fscanf(inputFile, "%lld", &arg);

            *((uint64_t*)codeStruct.codePointer + pc)     = PUSH;
            *((uint64_t*)codeStruct.codePointer + pc + 1) = arg;
            pc++;
        }

        else if (!strcmp(cmd, "add")){
            *((uint64_t*)codeStruct.codePointer + pc) = ADD;
            pc++;
        }

        else if (!strcmp(cmd, "sub")){
            *((uint64_t*)codeStruct.codePointer + pc) = SUB;
            pc++;
        }

        else if (!strcmp(cmd, "mul")){
            *((uint64_t*)codeStruct.codePointer + pc) = MUL;
            pc++;
        }

        else if (!strcmp(cmd, "div")){
            *((uint64_t*)codeStruct.codePointer + pc) = DIV;
            pc++;
        }

        else if (!strcmp(cmd, "sqrt")){
            *((uint64_t*)codeStruct.codePointer + pc) = SQRT;
            pc++;
        }

        else if (!strcmp(cmd, "sin")){
            *((uint64_t*)codeStruct.codePointer + pc) = SIN;
            pc++;
        }

        else if (!strcmp(cmd, "cos")){
            *((uint64_t*)codeStruct.codePointer + pc) = COS;
            pc++;
        }

        else if (!strcmp(cmd, "out")){
            *((uint64_t*)codeStruct.codePointer + pc) = OUT;
            pc++;
        }

        else if (!strcmp(cmd, "in")){
            *((uint64_t*)codeStruct.codePointer + pc) = IN;
            pc++;
        }

        else if (!strcmp(cmd, "dump")){
            *((uint64_t*)codeStruct.codePointer + pc) = DUMP;
            pc++;
        }

        else if (!strcmp(cmd, "hlt")){
            *((uint64_t*)codeStruct.codePointer + pc) = HLT;
            pc++;
            RunCommands = 0;
        }

        else{
            *((uint64_t*)codeStruct.codePointer + pc) = ERR;
            pc++;
        }
    }

    CommandsDump(&codeStruct);
    OutputCode(&codeStruct);

}

