#pragma once

#define DEF_CMD_(cmd, num,...) \
    CMD_##cmd = num,

const size_t MAX_NUM_COMMANDS = 512;
const size_t SIZE_COMMAND = 8;
const size_t SIZE_ARG = 8;

enum operations{
    #include "commands.hpp"
};

#undef DEF_CMD_
