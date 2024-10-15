#pragma once

enum errors{
    OK = 0,
    ERR_NULLPTR = 1,
    ERR = 2
};

typedef struct fileNames{
    const char* inputFileName;
    const char* outputFileName;
    const char* logFileName;

} fileNames_t;

typedef struct label{
    const char name[16];
    size_t addr;

} label_t;

typedef struct commands{
    const char*     name;

    void*           codePointer;
    size_t          numCommands;
    size_t          sizeArg;
    size_t          sizeAllocated;

    void*           labelsPointer;
    size_t          sizeLabels;


    fileNames_t*    fileNames;
    FILE*           logFile;
    FILE*           inputFile;
    FILE*           outputFile;
} commands;
