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

typedef struct header{

    int64_t     signature;
    int64_t     version;
    uint64_t    numCommands;

} header_t;

typedef struct fileNames{
    const char* inputFileName;
    const char* outputFileName;
    const char* logFileName;

} fileNames_t;

typedef struct string{
    size_t size;
    char*  addr;
} string_t;

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
    header_t        header;

    void*           codePointer;

    size_t          sizeAllocated;

    size_t          pc;

    label_t*        labelsPointer;
    size_t          sizeLabels;
    size_t          numElemsLabels;

    fixup_t*        fixupPointer;
    size_t          sizeFixup;
    size_t          numElemsFixup;

    void*           fileBuffer;
    size_t          sizeFileBuffer;
    string_t*       splittedInput;
    size_t          numSplitted;

    fileNames_t*    fileNames;
    FILE*           logFile;
    FILE*           inputFile;
    FILE*           outputFile;
    FILE*           outputBinFile;
} commands_t;
