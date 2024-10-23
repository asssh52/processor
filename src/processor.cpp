#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "../hpp/operations.hpp"
#include "../hpp/processor.hpp"
#include "../hpp/colors.hpp"

#define MEOW fprintf(stderr, "\e[0;31m" "\nmeow\n" "\e[0m");

const int       REGISTER_NUM    = 4;
const int       SIZE_RAM        = 256;
const int64_t   SIGNATURE       = 0x574f454d;
const int64_t   VERSION         = 5;
const int64_t   DRAW_RES_X      = 16;
const int64_t   DRAW_RES_Y      = 8;

typedef struct fileNames{

    const char* inputFileName;
    const char* outputFileName;
    const char* logFileName;

} fileNames_t;

typedef struct spu{
    const char*     name;

    Stack_t*        stk;
    Stack_t*        returnStack;
    int64_t*        RAM;
    void*           codePointer;
    void*           registersPointer;

    size_t          pc;

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
    OK_                 = 0,
    ERR_NULLPTR_        = 1,
    ERR_                = 2,
    INVALID_VERSION     = 3,
    INVALID_SIGNATURE   = 4
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
    const char* defaultFileNameIn  = "./bin/output_bin.asm";
    const char* defaultFileNameOut = "stdout";

    spu->inputFile  = fopen(spu->fileNames->inputFileName,  "r");
    if (spu->inputFile  == nullptr)     spu->inputFile = fopen(defaultFileNameIn, "r");

    spu->outputFile = fopen(spu->fileNames->outputFileName, "w");
    if (spu->outputFile == nullptr)     spu->outputFile = stdout;

    spu->logFile    = fopen(spu->fileNames->logFileName,    "w");
    if (spu->logFile    == nullptr)     spu->logFile = stdout;

    //INITIALIZE STACKS:
    spu->stk            = (Stack_t*)calloc(sizeof(Stack_t), 1);
    spu->returnStack    = (Stack_t*)calloc(sizeof(Stack_t), 1);
    StackCtor(spu->stk);                                             // check if allocated
    StackCtor(spu->returnStack);

    //FILL STRUCTURE FIELDS:
    spu->numRegisters   = REGISTER_NUM;
    spu->pc = 0;

    //VERIFY CODE:
    if (CheckSignature(spu)) return ERR_;

    //INITIALIZE CODE BUFFER:
    spu->codePointer            = calloc(SIZE_COMMAND, spu->numCommands); // check if allocated
    spu->memCommandsAllocated   = SIZE_COMMAND * spu->numCommands;

    //INITIALIZE REGISTER BUFFER:
    spu->registersPointer       = calloc(SIZE_ARG, spu->numRegisters);
    spu->memRegistersAllocated  = SIZE_ARG * spu->numRegisters;

    //INITIALIZE RAM:
    spu->RAM                    = (int64_t*)calloc(sizeof(int64_t), SIZE_RAM);



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
    free(spu->registersPointer);    //stack free
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
    header_t header = {};
    fread(&header, sizeof(header_t), 1, spu->inputFile);

    printf(BYEL "sign:%llx, ver:%lld, num:%lld\n" RESET, header.signature, header.version, header.numCommands);

    if (header.signature != SIGNATURE){
        spu->errorType = 4;

        return ERR_;
    }

    if (header.version != VERSION){
        spu->errorType = 3;

        return ERR_;
    }

    spu->numCommands = header.numCommands;

    return OK_;
}

/*=================================================================*/

static errors FillCodeBuffer(spu_t* spu){
                                                        //verificator
    fread(spu->codePointer, 1, spu->numCommands * 8, spu->inputFile);

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

static int64_t* GetPopValue(spu_t* spu, int64_t nextArg){

    int64_t argValue = 0;
    uint64_t returnValue = 0;
    uint64_t tempReturnValue = 0;

    //registers
    if (nextArg & 0b01000000){
        spu->pc++;
        argValue    = *((int64_t*)spu->registersPointer + *((int64_t*)spu->codePointer + spu->pc));
        returnValue =
               (size_t)((int64_t*)spu->registersPointer + *((int64_t*)spu->codePointer + spu->pc));
    }

    //immediate
    if (nextArg & 0b00100000){
        spu->pc++;
        argValue += *((int64_t*)spu->codePointer + spu->pc);
        *((int64_t*)spu->registersPointer) = argValue;
    }

    //memory
    if (nextArg & 0b10000000){
        returnValue = (size_t)(spu->RAM + argValue);
    }

    spu->pc++;

    if (!returnValue || (nextArg & 0b00100000 && nextArg & 0b01000000 && !(nextArg & 0b10000000))) return (int64_t*)spu->registersPointer;

    return (int64_t*)returnValue;
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
    fprintf(logFile, "Size of command:                      \t%lu\n", SIZE_COMMAND);
    fprintf(logFile, "Allocated memory for commands(bytes): \t%lu\n", spu->memCommandsAllocated);
    if (logFile == stdout) printf(RESET);

    fprintf(logFile, "\n");

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

        for (size_t ip = 0; ip < spu->numCommands; ip++){

            fprintf(spu->logFile, "pc<%0.2lu>: %lld", ip, *(cmdPtr + ip));

            if (ip == spu->pc){
                if (spu->logFile == stdout) fprintf(spu->logFile, BRED  "\t\t <---- ded" RESET);
                else                        fprintf(spu->logFile,       "\t\t <---- ded");
            }

            fprintf(spu->logFile, "\n");
        }
    }

    fprintf(logFile, "\n");

    if (logFile == stdout) printf(CYN);
    fprintf(logFile, "RAM size: %d\n", SIZE_RAM);
    if (logFile == stdout) printf(RESET);

    fprintf(logFile, "\n");

    if (logFile == stdout) printf(YEL);
    fprintf(logFile, "RAM:\n");
    if (logFile == stdout) printf(RESET);

    for (size_t adr = 0; adr < SIZE_RAM; adr++){
        fprintf(spu->logFile, "ram<%0.2lu>: %lld\n", adr, *(spu->RAM + adr));
    }

    fprintf(logFile, "---------------------------\n");

    //REGISTERS:
    if (logFile == stdout) printf(CYN);
    fprintf(logFile, "Number of registers:                  \t%lu\n", spu->numRegisters);
    fprintf(logFile, "Size of argument:                     \t%lu\n", SIZE_ARG);
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

static errors Draw1(spu_t* spu){
    printf("\npicture:\n\n");

                for (int x = 0; x < DRAW_RES_Y; x++){
                    for (int y = 0; y < DRAW_RES_X; y++){
                        if (spu->RAM[x * DRAW_RES_Y + y] == 0){
                            printf(".");
                        }

                        else{
                            printf("@");
                        }
                    }

                    printf("\n");
                }

                printf("\n");

    return OK_;
}

/*=================================================================*/

static errors Draw2(spu_t* spu){
    printf("\npicture:\n\n");
    int symbolsCount = 0;

    for (int i = 0; i < 100; i++){
        if (spu->RAM[2 * i] == 1){
            for (int j = 0; j < spu->RAM[2 * i + 1]; j++){
                printf(".");
                symbolsCount++;

                if (symbolsCount % DRAW_RES_X == 0){
                    printf("\n");
                }
            }
        }

        else if (spu->RAM[2 * i] == 2){
            for (int j = 0; j < spu->RAM[2 * i + 1]; j++){
                printf("@");
                symbolsCount++;

                if (symbolsCount % DRAW_RES_X == 0){
                    printf("\n");
                }
            }
        }
    }

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

    while (RunCommands){

        if (spu.pc > spu.numCommands){
            RunCommands = 0;
            ProcessorDump(&spu);
        }

        char* nextArg = (char*)spu.codePointer + spu.pc * SIZE_ARG;

        const char OPERATOR_MUSK = 0b00011111;
        switch (*nextArg & OPERATOR_MUSK){

            case PUSH:{

                StackPush(spu.stk, *GetPopValue(&spu, *nextArg));

                break;
            }

            case POP:{

                StackPop(spu.stk, GetPopValue(&spu, *nextArg));

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

            case JMP:{
                int64_t num_arg = 0;

                num_arg = *(nextArg + 1 * SIZE_COMMAND);

                spu.pc = num_arg;

                break;
            }

            case JA:{
                int64_t num_arg = 0, first_arg = 0, second_arg = 0;

                num_arg = *(nextArg + 1 * SIZE_COMMAND);

                StackPop(spu.stk, &first_arg);
                StackPop(spu.stk, &second_arg);

                if (first_arg > second_arg){
                    spu.pc = num_arg;

                    break;
                }

                else{
                    spu.pc += 2;

                    break;
                }
            }

            case CALL:{
                int64_t jump_to = 0;
                jump_to = *(nextArg + 1 * SIZE_COMMAND);
                StackPush(spu.returnStack, spu.pc);

                spu.pc = jump_to;
                break;
            }

            case RET:{
                int64_t num_arg = -1;
                StackPop(spu.returnStack, &num_arg);

                spu.pc = num_arg + 2;
                break;
            }

            case DRAW:{

                Draw2(&spu);

                spu.pc++;
                break;
            }

            case HLT:{
                RunCommands = 0;

                spu.pc++;
                break;
            }

            default :{
                printf(RED "\nERROR:pc=%lu\n" RESET, spu.pc);

                spu.pc++;
                break;
            }
        }
    }

    ProcessorDtor(&spu);
}

