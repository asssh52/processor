#include "../hpp/processor.hpp"

int main(int argc, const char *argv[]){
    const char* inputFile = (argc == 2) ? argv[1] : "/bin/user_input.asm";

    Run(inputFile);

    return 0;
}
