#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../hpp/colors.hpp"
#include "../hpp/compiler.hpp"
#include "../hpp/operations.hpp"

const size_t MAX_NUM_COMMANDS = 16;
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

    codeStruct->name = name;
    codeStruct->codePointer = calloc(codeStruct->sizeArg, codeStruct->numCommands);

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

    if (!codeStruct->codePointer){
        if (logFile == stdout) printf(RED);
        fprintf(logFile, "codeStruct does not exist\n");
        if (logFile == stdout) printf(RESET);
    }

    PrintFilesData(codeStruct);

    if (logFile == stdout) printf(CYN);
    fprintf(logFile, "Struct pointer:     \t%p\n",  codeStruct);
    fprintf(logFile, "Code pointer:       \t%p\n",  codeStruct->codePointer);
    fprintf(logFile, "Number of commands: \t%lu\n", codeStruct->numCommands);
    fprintf(logFile, "Size of command:    \t%lu\n", codeStruct->sizeArg);
    if (logFile == stdout) printf(RESET);

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

            //((uint64_t*)codeStruct.codePointer + pc);

            fprintf(outputFile, "%d %lld\n", PUSH, arg);
        }

        else if (!strcmp(cmd, "add")){
            fprintf(outputFile, "%d\n", ADD);
        }

        else if (!strcmp(cmd, "sub")){
            fprintf(outputFile, "%d\n", SUB);
        }

        else if (!strcmp(cmd, "mul")){
            fprintf(outputFile, "%d\n", MUL);
        }

        else if (!strcmp(cmd, "div")){
            fprintf(outputFile, "%d\n", DIV);
        }

        else if (!strcmp(cmd, "sqrt")){
            fprintf(outputFile, "%d\n", SQRT);
        }

        else if (!strcmp(cmd, "sin")){
            fprintf(outputFile, "%d\n", SIN);
        }

        else if (!strcmp(cmd, "cos")){
            fprintf(outputFile, "%d\n", COS);
        }

        else if (!strcmp(cmd, "out")){
            fprintf(outputFile, "%d\n", OUT);
        }

        else if (!strcmp(cmd, "in")){
            fprintf(outputFile, "%d\n", IN);
        }

        else if (!strcmp(cmd, "dump")){
            fprintf(outputFile, "%d\n", DUMP);
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

