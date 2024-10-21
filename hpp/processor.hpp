#pragma once

#include "/Users/asssh/Desktop/mystack/mystack.hpp"

typedef struct header{

    int64_t     signature;
    int64_t     version;
    uint64_t    numCommands;

} header_t;
