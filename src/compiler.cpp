#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "../hpp/colors.hpp"
#include "../hpp/compiler.hpp"
#include "../hpp/operations.hpp"

#define MEOW fprintf(stderr, "\e[0;31m" "\nmeow\n" "\e[0m");

const int64_t SIGNATURE = 0x574F454D;
const int64_t VERSION = 5;

const size_t MAX_LINES  = 64;
const size_t MAX_LABELS = 8;
const size_t MAX_FIXUP  = 8;

static void     Compile     (fileNames_t*   fileNames);
static errors   FillLabels  (commands_t*    codeStruct);
static errors   FillFixups  (commands_t*    codeStruct);
static int      FindLabel   (commands_t*    codeStruct, char* arg);
static errors   SplitCode   (commands_t*    codeStruct);

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
    const char* defaultFileNameIn       = "./bin/user_input.txt";
    const char* defaultFileNameOut      = "./bin/user_output.asm";
    const char* defaultFileNameOutBin   = "./bin/output_bin.asm";

    FILE* inputFile  = fopen(codeStruct->fileNames->inputFileName, "r");
    if (inputFile == nullptr)   inputFile = fopen(defaultFileNameIn, "r");

    FILE* outputFile = fopen(codeStruct->fileNames->outputFileName, "w");
    if (outputFile == nullptr)  outputFile = fopen(defaultFileNameOut, "w");

    FILE* outputBinFile = fopen(defaultFileNameOutBin, "wb");

    FILE* logFile = fopen(codeStruct->fileNames->logFileName, "w");
    if (logFile == nullptr) logFile = stdout;



    codeStruct->inputFile       = inputFile;
    codeStruct->outputFile      = outputFile;
    codeStruct->outputBinFile   = outputBinFile;
    codeStruct->logFile         = logFile;
    //files

    //HEADER:
    codeStruct->header.signature    = SIGNATURE;
    codeStruct->header.version      = VERSION;


    struct stat st;                                                   //change!
    stat(defaultFileNameIn, &st);
    size_t inputFileSize = st.st_size;

    codeStruct->fileBuffer      = calloc(1, inputFileSize);
    codeStruct->sizeFileBuffer  = inputFileSize;
    codeStruct->splittedInput   = (string_t*)calloc(sizeof(string_t), MAX_LINES + 1);
    codeStruct->numSplitted    = 0;

    fread(codeStruct->fileBuffer, 1, codeStruct->sizeFileBuffer, inputFile);
    SplitCode(codeStruct);

    printf(BBLU "%lu" RESET, codeStruct->numSplitted);

    codeStruct->name = name;
    codeStruct->codePointer     = calloc(SIZE_ARG, MAX_NUM_COMMANDS);     //exception nlptr = calloc
    codeStruct->sizeAllocated   = MAX_NUM_COMMANDS * SIZE_ARG;

    codeStruct->numElemsFixup   = 0;
    codeStruct->fixupPointer    = (fixup_t*)calloc(sizeof(fixup_t), MAX_FIXUP);
    FillFixups(codeStruct);

    codeStruct->numElemsLabels  = 0;
    codeStruct->labelsPointer   = (label_t*)calloc(sizeof(label_t), MAX_LABELS);
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
    fprintf(logFile, "Number of commands:                   \t%lu\n", MAX_NUM_COMMANDS);
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
    fprintf(logFile, "Size of labels:\t\t%lu\n",  MAX_LABELS);

    label_t* labelsPtr = codeStruct->labelsPointer;
    for (int i = 0; i < MAX_LABELS; i++){
        fprintf(logFile, "[label:%d], name:<%s>, addr:%lld\n", i, (labelsPtr + i)->name, (labelsPtr + i)->addr);
    }
    if (logFile == stdout) printf(RESET);


    fprintf(logFile, "\n");


    //FIXUP:
    if (logFile == stdout) printf(CYN);
    fprintf(logFile, "Fixup data.\n");
    fprintf(logFile, "Fixup pointer:\t\t%p\n",          codeStruct->fixupPointer);
    fprintf(logFile, "Size of fixups:\t\t%lu\n",          MAX_FIXUP);
    fprintf(logFile, "Number of fixups:\t%lu\n",        codeStruct->numElemsFixup);

    fixup_t* fixupsPtr = codeStruct->fixupPointer;
    for (int i = 0; i < MAX_FIXUP; i++){
        fprintf(logFile, "[fixup:%d], label name:<%lld>, code addr:%lld\n",
                                 i, (fixupsPtr + i)->labelNum, (fixupsPtr + i)->codeAdr);
    }
    if (logFile == stdout) printf(RESET);


    fprintf(logFile, "\n");

    //SPLITTED:
    if (logFile == stdout) printf(BLU);
    fprintf(logFile, "Splitted code:\n");
    for (int i = 0; i < codeStruct->numSplitted; i++){
        printf("i:%d - \t%p \t size:%lu\n", i + 1, (codeStruct->splittedInput)[i].addr, (codeStruct->splittedInput)[i].size);
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

    fprintf(outputFile, "LISTING:\n");
    fprintf(outputFile, "signature: %llx\n", SIGNATURE);
    fprintf(outputFile, "version: v.%lld\n", VERSION);
    fprintf(outputFile, "num commands:%lu\n", codeStruct->pc);

    return OK;
}

/*=======================================================================*/

static errors OutputCodeListing(commands_t* codeStruct){
    if (!codeStruct || !codeStruct->codePointer) return ERR_NULLPTR;
                                                                           //add file verifycator
    int64_t* cmdPtr = (int64_t*)(codeStruct->codePointer);                 // change type

    for (size_t pc = 0; pc < MAX_NUM_COMMANDS; pc++){
        fprintf(codeStruct->outputFile, "%lld\n", *(cmdPtr + pc));
    }

    return OK;
}

/*=======================================================================*/

static errors OutputCodeBin(commands_t* codeStruct){
    if (!codeStruct || !codeStruct->codePointer) return ERR_NULLPTR;
                                      //add file verifycator
    codeStruct->header.numCommands = codeStruct->pc;

    fwrite(&codeStruct->header      , sizeof(header_t), 1                             , codeStruct->outputBinFile);
    fwrite(codeStruct->codePointer  , sizeof(int64_t) , codeStruct->header.numCommands, codeStruct->outputBinFile);

    return OK;
}

/*=======================================================================*/
                                                                                //capital letters
int FindRegisterName(char* arg){

    return arg[0] - 'a' + 1;

}

/*=======================================================================*/

int CheckMark(commands_t* codeStruct, char* arg, checkMarkParams param){   //rename

    char* ptr = strchr(arg, ':');
    int hasMark = (ptr)? 1:0;
    printf(BRED "hasMark:%d %s\n" RESET, hasMark, arg);

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

    for (int i = 0; i < MAX_LABELS; i++){
        (codeStruct->labelsPointer + i)->addr = -666;
        strcpy((codeStruct->labelsPointer + i)->name, "meow");
    }

    return OK;
}

/*=======================================================================*/

static errors FillFixups(commands_t* codeStruct){

    for (int i = 0; i < MAX_FIXUP; i++){
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

static char* GetWord(commands_t* codeStruct, int numLine, int wordSize, char* word){
    string_t line   = codeStruct->splittedInput[numLine];
    char* startAddr = nullptr;

    if (!numLine)   startAddr = line.addr;
    else            startAddr = line.addr + 1;

    for (int i = 0; i < line.size - 1; i++){
        if (startAddr[i] == '\n' || startAddr[i] == ' '){
            word[i] = '\0';

            return startAddr + i + 1;
        }

        word[i] = startAddr[i];
    }

    return startAddr;
}

/*=======================================================================*/

static char* GetArg(char* input, char* arg){

    for (int i = 0; i < 32; i++){
        if (input[i] == '\n' || input[i] == ' '){
            arg[i] = '\0';

            return arg + i + 1;
        }

        arg[i] = input[i];
    }


    return arg;
}

/*=======================================================================*/

static errors SplitCode(commands_t* codeStruct){
    codeStruct->splittedInput->addr = (char*)codeStruct->fileBuffer;

    for (int i = 0; i < codeStruct->sizeFileBuffer; i++){
        char arg = ((char*)codeStruct->fileBuffer)[i];

        if (arg == '\n'){
            (codeStruct->splittedInput)[codeStruct->numSplitted + 1].addr   = (char*)(codeStruct->fileBuffer) + i;
            char* lastAdr = (char*)(codeStruct->fileBuffer) + i;
            char* prevAdr = (codeStruct->splittedInput)[codeStruct->numSplitted].addr;

            (codeStruct->splittedInput)[codeStruct->numSplitted].size = lastAdr - prevAdr;
            codeStruct->numSplitted++;
            printf(BGRN "<%p>\n" RESET, (codeStruct->splittedInput)[codeStruct->numSplitted].addr);
            printf(BGRN "<%lu>\n" RESET, (codeStruct->splittedInput)[codeStruct->numSplitted - 1].size);
        }
    }

    return OK;
}

/*=======================================================================*/
static void Compile(fileNames_t* fileNames){
    commands_t codeStruct = {};
    codeStruct.fileNames = fileNames;

    CommandsCtor(&codeStruct, "mycode");

    CommandsDump(&codeStruct);

    FILE* inputFile  = codeStruct.inputFile;
    FILE* outputFile = codeStruct.outputFile;

    bool RunCommands    = 1;
    int numLine         = 0;

    while (RunCommands){
        char cmd[128] = "";
        if (numLine >= codeStruct.numSplitted) break;

        char* second_cmdPtr = GetWord(&codeStruct, numLine, 128, cmd);
        printf(BMAG "<%s>\n" RESET, cmd);
        printf(BMAG "<%c>\n" RESET, *second_cmdPtr);

        if (!strcmp(cmd, "push")){
            int64_t numArg = 0;
            int64_t numReg = 0;

            char secondArg[32]  = "";
            GetArg(second_cmdPtr, secondArg);

            printf("%s\n", secondArg);

            char* ptrMemory     = strchr(secondArg,'[');
            char* ptrRegisters  = strchr(secondArg,'x');
            char* ptrSum        = strchr(secondArg,'+');
            bool mem    = (ptrMemory)       ?1:0;
            bool reg    = (ptrRegisters)    ?1:0;
            bool sum    = (ptrSum)          ?1:0;
            char switchValue    = reg * 2 + sum;

            printf(BBLU "r:%p, s:%p, m:%p\n" RESET, ptrRegisters, ptrSum, ptrMemory);

            switch (switchValue){
                case 0:{
                    numArg = atol(secondArg);
                    if (mem) numArg = atol(ptrMemory + 1);


                    *((uint64_t*)codeStruct.codePointer + codeStruct.pc)            = 0b00100001;
                    if (mem) *((uint64_t*)codeStruct.codePointer + codeStruct.pc)   = 0b10100001;

                    *((uint64_t*)codeStruct.codePointer + codeStruct.pc + 1)= numArg;
                    codeStruct.pc += 2;
                    break;
                }

                case 2:{
                    numReg = FindRegisterName(ptrRegisters - 1);


                    *((uint64_t*)codeStruct.codePointer + codeStruct.pc)            = 0b01000001;
                    if (mem) *((uint64_t*)codeStruct.codePointer + codeStruct.pc)   = 0b11000001;

                    *((uint64_t*)codeStruct.codePointer + codeStruct.pc + 1)= numReg;
                    codeStruct.pc += 2;
                    break;
                }

                case 3:{
                    numArg = atol(ptrSum + 1);
                    numReg = FindRegisterName(ptrRegisters - 1);


                    *((uint64_t*)codeStruct.codePointer + codeStruct.pc)            = 0b01100001;
                    if (mem) *((uint64_t*)codeStruct.codePointer + codeStruct.pc)   = 0b11100001;

                    *((uint64_t*)codeStruct.codePointer + codeStruct.pc + 1)= numReg;
                    *((uint64_t*)codeStruct.codePointer + codeStruct.pc + 2)= numArg;
                    codeStruct.pc += 3;
                    break;
                }

                default:{
                    RunCommands = 0;
                    printf(BRED "\nERROR\n\n" RESET);
                    break;
                }
            }
        }

        else if (!strcmp(cmd, "pop")){
            int64_t numArg = 0;
            int64_t numReg = 0;

            char secondArg[32]  = "";
            GetArg(second_cmdPtr, secondArg);

            char* ptrMemory     = strchr(secondArg,'[');
            char* ptrRegisters  = strchr(secondArg,'x');
            char* ptrSum        = strchr(secondArg,'+');
            bool mem    = (ptrMemory)       ?1:0;
            bool reg    = (ptrRegisters)    ?1:0;
            bool sum    = (ptrSum)          ?1:0;
            char switchValue = mem * 4 + reg * 2 + sum;

            switch(switchValue){
                case 2:{
                    numReg = FindRegisterName(ptrRegisters - 1);


                    *((uint64_t*)codeStruct.codePointer + codeStruct.pc)    = 0b01001001;
                    *((uint64_t*)codeStruct.codePointer + codeStruct.pc + 1)= numReg;
                    codeStruct.pc += 2;
                    break;
                }

                case 4:{
                    numArg = atol(ptrMemory + 1);


                    *((uint64_t*)codeStruct.codePointer + codeStruct.pc)    = 0b10101001;
                    *((uint64_t*)codeStruct.codePointer + codeStruct.pc + 1)= numArg;
                    codeStruct.pc += 2;
                    break;
                }

                case 6:{
                    numReg = FindRegisterName(ptrRegisters - 1);


                    *((uint64_t*)codeStruct.codePointer + codeStruct.pc)    = 0b11001001;
                    *((uint64_t*)codeStruct.codePointer + codeStruct.pc + 1)= numReg;
                    codeStruct.pc += 2;
                    break;
                }

                case 7:{
                    numArg = atol(ptrSum + 1);
                    numReg = FindRegisterName(ptrRegisters - 1);


                    *((uint64_t*)codeStruct.codePointer + codeStruct.pc)    = 0b11101001;
                    *((uint64_t*)codeStruct.codePointer + codeStruct.pc + 1)= numReg;
                    *((uint64_t*)codeStruct.codePointer + codeStruct.pc + 2)= numArg;
                    codeStruct.pc += 3;
                    break;
                }

                default:{
                    RunCommands = 0;
                    printf(BRED "\nERROR\n\n" RESET);
                    break;
                }
            }
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

        else if (!strcmp(cmd, "draw")){
            *((uint64_t*)codeStruct.codePointer + codeStruct.pc) = DRAW;
            codeStruct.pc++;
        }

        else if (!strcmp(cmd, "jmp")){
            int64_t numArg = 0;
            char secondArg[32]  = "";
            GetArg(second_cmdPtr, secondArg);

            char* ptr = strchr(secondArg, ':');

            if (ptr) numArg = CheckMark(&codeStruct, secondArg, FROM_FUNC);
            else     numArg = atol(secondArg);

            *((uint64_t*)codeStruct.codePointer + codeStruct.pc)        = JMP;
            *((uint64_t*)codeStruct.codePointer + codeStruct.pc + 1)    = numArg;
            codeStruct.pc += 2;
        }

        else if (!strcmp(cmd, "ja")){
            int64_t numArg = 0;
            char secondArg[32]  = "";
            GetArg(second_cmdPtr, secondArg);

            char* ptr = strchr(secondArg, ':');

            if (ptr){
                numArg = CheckMark(&codeStruct, secondArg, FROM_FUNC);
            }

            else {
                numArg = atol(secondArg);
            }

            *((uint64_t*)codeStruct.codePointer + codeStruct.pc)        = JA;
            *((uint64_t*)codeStruct.codePointer + codeStruct.pc + 1)    = numArg;
            codeStruct.pc += 2;
        }

        else if (!strcmp(cmd, "hlt")){
            *((uint64_t*)codeStruct.codePointer + codeStruct.pc) = HLT;
            codeStruct.pc++;
        }

        else if (!strcmp(cmd, "call")){
            int64_t numArg = 0;
            char secondArg[32]  = "";
            GetArg(second_cmdPtr, secondArg);

            numArg = CheckMark(&codeStruct, secondArg, FROM_FUNC);

            *((uint64_t*)codeStruct.codePointer + codeStruct.pc)        = CALL;
            *((uint64_t*)codeStruct.codePointer + codeStruct.pc + 1)    = numArg;
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

        numLine++;
    }

    CommandsDump(&codeStruct);
    FixCode(&codeStruct);

    OutputCodeBin(&codeStruct);

    PrintSignature(&codeStruct);
    OutputCodeListing(&codeStruct);
}

