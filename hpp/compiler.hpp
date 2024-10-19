#pragma once

enum errors{
    OK = 0,
    ERR_NULLPTR = 1,
    ERR = -666
};

enum checkMarkParams{
    FROM_CODE = 0,
    FROM_FUNC = 1
};

typedef struct fileNames{
    const char* inputFileName;
    const char* outputFileName;
    const char* logFileName;

} fileNames_t;

typedef struct label{
    char    name[64];
    int64_t  addr;

} label_t;

typedef struct fixup{
    int64_t  codeAdr;
    int64_t  labelNum;

} fixup_t;

typedef struct commands{
    const char*     name;

    void*           codePointer;
    size_t          numCommands;
    size_t          sizeAllocated;

    size_t          maxLines;
    size_t          numLines;

    size_t          pc;

    label_t*        labelsPointer;      //приведение типа
    size_t          sizeLabels;
    size_t          numElemsLabels;

    fixup_t*        fixupPointer;
    size_t          sizeFixup;
    size_t          numElemsFixup;

    fileNames_t*    fileNames;
    FILE*           logFile;
    FILE*           inputFile;
    FILE*           outputFile;
} commands_t;
