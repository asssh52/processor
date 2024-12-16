#pragma once

const size_t MAX_NUM_COMMANDS = 512;
const size_t SIZE_COMMAND = 8;
const size_t SIZE_ARG = 8;

enum operations{
    PUSH    = 1,
    ADD     = 2,
    SUB     = 3,
    MUL     = 4,
    DIV     = 5,

    SQRT    = 6,
    SIN     = 7,
    COS     = 8,

    POP     = 9,
    OUT     = 10,
    IN      = 11,
    DUMP    = 12,
    JMP     = 13,
    JA      = 14,
    JAE     = 15,
    JE      = 16,
    JNE     = 17,

    HLT     = 18,

    CALL    = 19,
    RET     = 20,
    DRAW    = 21,
    MOD     = 22,

    LS_EQ   = 23,
    MR_EQ   = 24,
    EQL     = 25,
    LS      = 26,
    MR      = 27,

};
