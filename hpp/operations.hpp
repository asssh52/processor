#pragma once

const size_t MAX_NUM_COMMANDS = 16;
const size_t SIZE_COMMAND = 8;

enum operations{
    PUSH =  1,
    ADD  =  2,
    SUB  =  3,
    MUL  =  4,
    DIV  =  5,

    SQRT =  10,
    SIN  =  11,
    COS  =  12,

    OUT  =  20,
    IN   =  21,
    DUMP =  22,
    HLT  = -1,
};
