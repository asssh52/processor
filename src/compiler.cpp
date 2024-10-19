#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../hpp/colors.hpp"
#include "../hpp/compiler.hpp"
#include "../hpp/operations.hpp"

#define MEOW fprintf(stderr, "\e[0;31m" "\nmeow\n" "\e[0m");

const char* SIGNATURE = "meow";
const int VERSION = 4;

const size_t MAX_LINES  = 32;
const size_t NUM_LABELS = 8;
const size_t NUM_FIXUP  = 8;

static void     Compile     (fileNames_t*   fileNames);
static errors   FillLabels  (commands_t*    codeStruct);
static errors   FillFixups  (commands_t*    codeStruct);
static int      FindLabel   (commands_t*    codeStruct, char* arg);

int main(int argc, const char* argv[]){
    fileNames_t fileNames= {};

    fileNames.inputFileName  = (argc == 3) ? argv[1]  : "./bin/user_input.txt";
    fileNames.outputFileName = (argc == 3) ? argv[2]  : "./bin/user_output.asm";

    Compile(&fileNames);

    return 0;
}

/*=======================================================================*/

static errors PrintFilesData(commands_t* codeStruct){
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

static errors CommandsCtor(commands_t* codeStruct, const char* name){
    if (!codeStruct) return ERR_NULLPTR;

    //files
    const char* defaultFileNameIn  = "./bin/user_input.txt";
    const char* defaultFileNameOut = "./bin/user_output.asm";

    FILE* inputFile  = fopen(codeStruct->fileNames->inputFileName, "r");
    if (inputFile == nullptr)   inputFile = fopen(defaultFileNameIn, "r");

    FILE* outputFile = fopen(codeStruct->fileNames->outputFileName, "w");
    if (outputFile == nullptr)  outputFile = fopen(defaultFileNameOut, "w");

    FILE* logFile = fopen(codeStruct->fileNames->logFileName, "w");
    if (logFile == nullptr) logFile = stdout;

    codeStruct->inputFile   = inputFile;
    codeStruct->outputFile  = outputFile;
    codeStruct->logFile     = logFile;
    //files

    codeStruct->numCommands     = MAX_NUM_COMMANDS;                                         //count commands
    codeStruct->maxLines        = MAX_LINES;

    codeStruct->name = name;
    codeStruct->codePointer     = calloc(SIZE_ARG, codeStruct->numCommands);     //exception nlptr = calloc
    codeStruct->sizeAllocated   = codeStruct->numCommands * SIZE_ARG;

    codeStruct->numElemsFixup   = 0;
    codeStruct->sizeFixup       = NUM_FIXUP;
    codeStruct->fixupPointer    = (fixup_t*)calloc(sizeof(fixup_t), codeStruct->sizeFixup);
    FillFixups(codeStruct);

    codeStruct->sizeLabels      = NUM_LABELS;
    codeStruct->numElemsLabels  = 0;
    codeStruct->labelsPointer   = (label_t*)calloc(sizeof(label_t), codeStruct->sizeLabels);
    FillLabels(codeStruct);

    return OK;
}

/*=======================================================================*/

static errors CommandsDtor(commands_t* codeStruct){                                         //free pointers
    if (!codeStruct->codePointer) return ERR_NULLPTR;

    free(codeStruct->codePointer);

    if (!codeStruct->logFile && codeStruct->logFile != stdout) fclose(codeStruct->logFile);
    if (!codeStruct->inputFile)                                fclose(codeStruct->inputFile);
    if (!codeStruct->outputFile)                               fclose(codeStruct->outputFile);

    fprintf(codeStruct->logFile, CYN "%s destoyed\n"  RESET, codeStruct->name);

    return OK;
}

/*=======================================================================*/

static errors CommandsDump(commands_t* codeStruct){
    if (!codeStruct->logFile) codeStruct->logFile = stdout;
        FILE* outputFile = codeStruct->logFile;
    FILE* logFile = codeStruct->logFile;

    fprintf(logFile, "=================================================\n");

    if (logFile == stdout) printf(GRN);
    fprintf(logFile, "dump of \"%s\":\n", codeStruct->name);
    if (logFile == stdout) printf(RESET);

    PrintFilesData(codeStruct);


    //STRUCT DATA:
    if (logFile == stdout) printf(CYN);
    fprintf(logFile, "Struct pointer:                       \t%p\n",  codeStruct);
    fprintf(logFile, "Code pointer:                         \t%p\n",  codeStruct->codePointer);
    fprintf(logFile, "Number of commands:                   \t%lu\n", codeStruct->numCommands);
    fprintf(logFile, "Size of command:                      \t%lu\n", SIZE_ARG);
    fprintf(logFile, "Allocated memory for commands(bytes): \t%lu\n", codeStruct->sizeAllocated);
    if (logFile == stdout) printf(RESET);

    if (!codeStruct->codePointer){
        if (logFile == stdout) printf(RED);
        fprintf(logFile, "codeStruct does not exist\n");
        if (logFile == stdout) printf(RESET);
    }

    fprintf(logFile, "\n");


    //LABEL DATA:
    if (logFile == stdout) printf(BLU);
    fprintf(logFile, "Label data.\n");
    fprintf(logFile, "Label pointer:\t\t%p\n",  codeStruct->labelsPointer);
    fprintf(logFile, "Size of labels:\t%lu\n",  codeStruct->sizeLabels);

    label_t* labelsPtr = codeStruct->labelsPointer;
    for (int i = 0; i < codeStruct->sizeLabels; i++){
        fprintf(logFile, "[label:%d], name:<%s>, addr:%lld\n", i, (labelsPtr + i)->name, (labelsPtr + i)->addr);
    }
    if (logFile == stdout) printf(RESET);


    fprintf(logFile, "\n");


    //FIXUP:
    if (logFile == stdout) printf(CYN);
    fprintf(logFile, "Fixup data.\n");
    fprintf(logFile, "Fixup pointer:\t\t%p\n",          codeStruct->fixupPointer);
    fprintf(logFile, "Size of fixups:\t%lu\n",          codeStruct->sizeFixup);
    fprintf(logFile, "Number of fixups:\t%lu\n",        codeStruct->numElemsFixup);

    fixup_t* fixupsPtr = codeStruct->fixupPointer;
    for (int i = 0; i < codeStruct->sizeFixup; i++){
        fprintf(logFile, "[fixup:%d], label name:<%lld>, code addr:%lld\n",
                                 i, (fixupsPtr + i)->labelNum, (fixupsPtr + i)->codeAdr);
    }
    if (logFile == stdout) printf(RESET);


    fprintf(logFile, "\n");


    //COMMANDS:
    int64_t* cmdPtr = (int64_t*)(codeStruct->codePointer);                           // change type

    if (logFile == stdout) printf(GRN);
    fprintf(logFile, "Commands:\n");
    if (logFile == stdout) printf(RESET);

    for (size_t pc = 0; pc < MAX_NUM_COMMANDS; pc++){
            fprintf(codeStruct->logFile, "pc<%lu>:\t%lld\n", pc, *(cmdPtr + pc));
    }
    fprintf(logFile, "=================================================\n");

    if (logFile == stdout) getchar();

    return OK;
}

/*=======================================================================*/

static errors PrintSignature(commands_t* codeStruct){
    FILE* outputFile = codeStruct->outputFile;

    fprintf(outputFile, "=======================================\n");
    fprintf(outputFile, "signature: %s\n", SIGNATURE);
    fprintf(outputFile, "version: v.%d\n", VERSION);
    fprintf(outputFile, "=======================================\n");

    return OK;
}

/*=======================================================================*/

static errors OutputCode(commands_t* codeStruct){
    if (!codeStruct || !codeStruct->codePointer) return ERR_NULLPTR;
                                                                                        //add file verifycator

    int64_t* cmdPtr = (int64_t*)(codeStruct->codePointer);                            // change type

    for (size_t pc = 0; pc < MAX_NUM_COMMANDS; pc++){
        fprintf(codeStruct->outputFile, "%lld\n", *(cmdPtr + pc));
    }

    return OK;
}

/*=======================================================================*/
                                                                                //capital letters
int FindRegisterName(char arg[3]){

    return arg[0] - 'a' + 1;

}

/*=======================================================================*/

int CheckMark(commands_t* codeStruct, char* arg, checkMarkParams param){        //rename

    char* ptr = strchr(arg, ':');
    int hasMark = (ptr)? 1:0;
    printf(BRED "%s\n" RESET, arg);

    if (param == FROM_CODE){
        if (hasMark){
            if (ptr) *ptr = '\0';
            int labelNum = FindLabel(codeStruct, arg);

            if (labelNum == -1){
                strcpy((codeStruct->labelsPointer + codeStruct->numElemsLabels)->name, arg);
                       (codeStruct->labelsPointer + codeStruct->numElemsLabels)->addr  = codeStruct->pc;

                codeStruct->numElemsLabels++;

                return hasMark;
            }

            else{
                (codeStruct->labelsPointer + labelNum)->addr  = codeStruct->pc;

                return hasMark;
            }
        }
    }

    else if (param == FROM_FUNC){
        if (hasMark){
            if (ptr) *ptr = '\0';
            int labelNum = FindLabel(codeStruct, arg);

            if (labelNum == -1){
                strcpy((codeStruct->labelsPointer + codeStruct->numElemsLabels)->name, arg);
                (codeStruct->labelsPointer + codeStruct->numElemsLabels)->addr  = -1;



                //FIXUP:
                fixup_t* fxp_ptr = codeStruct->fixupPointer;

                (fxp_ptr + codeStruct->numElemsFixup)->labelNum = codeStruct->numElemsLabels;
                (fxp_ptr + codeStruct->numElemsFixup)->codeAdr  = codeStruct->pc + 1;
                codeStruct->numElemsLabels++;
                codeStruct->numElemsFixup++;
                return -1;
            }

            else{
                return (codeStruct->labelsPointer + labelNum)->addr;
            }
        }

        else{
            return ERR;
        }
    }

    else{
        return hasMark;
    }

    return ERR;
}

/*=======================================================================*/

static int FindLabel(commands_t* codeStruct, char* arg){

    for (int i = 0; i < codeStruct->numElemsLabels; i++){
        if (!strcmp((codeStruct->labelsPointer + i)->name, arg)){
            return i;
        }
    }

    return -1;
}

/*=======================================================================*/

static errors FillLabels(commands_t* codeStruct){

    for (int i = 0; i < codeStruct->sizeLabels; i++){
        (codeStruct->labelsPointer + i)->addr = -666;
        strcpy((codeStruct->labelsPointer + i)->name, "meow");
    }

    return OK;
}

/*=======================================================================*/

static errors FillFixups(commands_t* codeStruct){

    for (int i = 0; i < codeStruct->sizeFixup; i++){
        (codeStruct->fixupPointer + i)->  codeAdr = -666;
        (codeStruct->fixupPointer + i)->labelNum  = -666;
    }

    return OK;
}

/*=======================================================================*/

static errors FixCode(commands_t* codeStruct){

    for (int i = 0; i < codeStruct->numElemsFixup; i++){
        int64_t  codeAddr    = (codeStruct->fixupPointer + i)->codeAdr;
        int64_t  labelNum    = (codeStruct->fixupPointer + i)->labelNum;

        int64_t codeArgChangeTo   = (codeStruct->labelsPointer + labelNum)->addr;

        *((int64_t*)codeStruct->codePointer + codeAddr) = codeArgChangeTo;                              //int64_t!!
    }

    return OK;
}

/*=======================================================================*/

static int64_t GetArg(commands_t* codeStruct){
    int64_t arg         = 0;
    char    arg_ch[64]  = "";
        if (!fscanf(codeStruct->inputFile,  "%lld", &arg)){
            fscanf(codeStruct->inputFile,   "%s", arg_ch);

            arg = CheckMark(codeStruct, arg_ch, FROM_FUNC);
        }

    return arg;
}

/*=======================================================================*/

static void Compile(fileNames_t* fileNames){
    commands_t codeStruct = {};
    codeStruct.fileNames = fileNames;

    CommandsCtor(&codeStruct, "mycode");

    CommandsDump(&codeStruct);

    PrintSignature(&codeStruct);

    FILE* inputFile  = codeStruct.inputFile;
    FILE* outputFile = codeStruct.outputFile;

    bool RunCommands = 1;

    while (RunCommands){
        char cmd[128] = "";
        fscanf(inputFile, "%s", cmd);


        if (!strcmp(cmd, "push")){
            int64_t arg         = 0;
            int64_t arg2        = 0;
            char    reg_name[3] = "";

            int firstArgExist = fscanf(inputFile, "%lld", &arg);
            if (!firstArgExist){
                                        fscanf(inputFile, "%s", reg_name);
                int secondArgExist =    fscanf(inputFile, " + %lld", &arg2);

                if (!secondArgExist){
                    *((int64_t*)codeStruct.codePointer + codeStruct.pc)     = 0b01000001;

                    arg = FindRegisterName(reg_name);

                    *((int64_t*)codeStruct.codePointer + codeStruct.pc + 1) = arg;

                    codeStruct.pc += 2;
                }

                else{
                    *((int64_t*)codeStruct.codePointer + codeStruct.pc)     = 0b01100001;

                    arg = FindRegisterName(reg_name);

                    *((int64_t*)codeStruct.codePointer + codeStruct.pc + 1) = arg;
                    *((int64_t*)codeStruct.codePointer + codeStruct.pc + 2) = arg2;

                    codeStruct.pc += 3;
                }
            }

            else{

                *((int64_t*)codeStruct.codePointer + codeStruct.pc)     = 0b00100001;
                *((int64_t*)codeStruct.codePointer + codeStruct.pc + 1) = arg;

                codeStruct.pc += 2;

            }
        }

        else if (!strcmp(cmd, "pop")){
            char reg_name[3] = "";
            int64_t arg = 0;

            fscanf(inputFile, "%s", reg_name);
            arg = FindRegisterName(reg_name);

            *((uint64_t*)codeStruct.codePointer + codeStruct.pc) = POP;
            *((uint64_t*)codeStruct.codePointer + codeStruct.pc + 1) = arg;

            codeStruct.pc += 2;
        }

        else if (!strcmp(cmd, "add")){
            *((uint64_t*)codeStruct.codePointer + codeStruct.pc) = ADD;
            codeStruct.pc++;
        }

        else if (!strcmp(cmd, "sub")){
            *((uint64_t*)codeStruct.codePointer + codeStruct.pc) = SUB;
            codeStruct.pc++;
        }

        else if (!strcmp(cmd, "mul")){
            *((uint64_t*)codeStruct.codePointer + codeStruct.pc) = MUL;
            codeStruct.pc++;
        }

        else if (!strcmp(cmd, "div")){
            *((uint64_t*)codeStruct.codePointer + codeStruct.pc) = DIV;
            codeStruct.pc++;
        }

        else if (!strcmp(cmd, "sqrt")){
            *((uint64_t*)codeStruct.codePointer + codeStruct.pc) = SQRT;
            codeStruct.pc++;
        }

        else if (!strcmp(cmd, "sin")){
            *((uint64_t*)codeStruct.codePointer + codeStruct.pc) = SIN;
            codeStruct.pc++;
        }

        else if (!strcmp(cmd, "cos")){
            *((uint64_t*)codeStruct.codePointer + codeStruct.pc) = COS;
            codeStruct.pc++;
        }

        else if (!strcmp(cmd, "out")){
            *((uint64_t*)codeStruct.codePointer + codeStruct.pc) = OUT;
            codeStruct.pc++;
        }

        else if (!strcmp(cmd, "in")){
            *((uint64_t*)codeStruct.codePointer + codeStruct.pc) = IN;
            codeStruct.pc++;
        }

        else if (!strcmp(cmd, "dump")){
            *((uint64_t*)codeStruct.codePointer + codeStruct.pc) = DUMP;
            codeStruct.pc++;
        }

        else if (!strcmp(cmd, "jmp")){
            *((uint64_t*)codeStruct.codePointer + codeStruct.pc) = JMP;

            //fixup
            int64_t arg = GetArg(&codeStruct);
            //fixup

            *((uint64_t*)codeStruct.codePointer + codeStruct.pc + 1) = arg;
            codeStruct.pc += 2;
        }

        else if (!strcmp(cmd, "ja")){
            *((uint64_t*)codeStruct.codePointer + codeStruct.pc) = JA;

            //fixup
            int64_t arg = GetArg(&codeStruct);
            //fixup

            *((uint64_t*)codeStruct.codePointer + codeStruct.pc + 1) = arg;
            codeStruct.pc += 2;
        }

        else if (!strcmp(cmd, "hlt")){
            *((uint64_t*)codeStruct.codePointer + codeStruct.pc) = HLT;
            codeStruct.pc++;
        }

        else if (!strcmp(cmd, "call")){
            *((uint64_t*)codeStruct.codePointer + codeStruct.pc) = CALL;
            int64_t arg = GetArg(&codeStruct);
            //fixup

            *((uint64_t*)codeStruct.codePointer + codeStruct.pc + 1) = arg;
            codeStruct.pc += 2;
        }

        else if (!strcmp(cmd, "ret")){
            *((uint64_t*)codeStruct.codePointer + codeStruct.pc) = RET;
            codeStruct.pc++;
        }

        else{
            if (!CheckMark(&codeStruct, cmd, FROM_CODE)){

                *((uint64_t*)codeStruct.codePointer + codeStruct.pc) = ERR; //!!!
            }
        }

        if (codeStruct.numLines >= codeStruct.maxLines) RunCommands = 0;
        codeStruct.numLines++;

    }

    FixCode(&codeStruct);
    CommandsDump(&codeStruct);
    OutputCode(&codeStruct);

}

