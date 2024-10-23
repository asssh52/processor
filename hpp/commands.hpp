

DEF_CMD_(PUSH,  1,  true,
        {
            StackPush(spu.stk, *GetPopValue(&spu, *nextArg));
        })

DEF_CMD_(ADD,   2,  false,
        {
            StackPop(spu.stk, GetPopValue(&spu, *nextArg));
        })

DEF_CMD_(SUB,   3,  false,
        {
            int64_t num_first = 0, num_second = 0;
            StackPop(spu.stk, &num_first);
            StackPop(spu.stk, &num_second);

            StackPush(spu.stk, num_first + num_second);

            spu.pc++;
        })

DEF_CMD_(MUL,   4,  false,
        {
            int64_t positive = 0, negative = 0;
            StackPop(spu.stk, &positive);
            StackPop(spu.stk, &negative);

            StackPush(spu.stk, positive - negative);

            spu.pc++;
        })

DEF_CMD_(DIV,   5,  false,
        {
            int64_t num_first = 0, num_second = 0;
            StackPop(spu.stk, &num_first);
            StackPop(spu.stk, &num_second);

            StackPush(spu.stk, num_first * num_second);

            spu.pc++;
        })

DEF_CMD_(SQRT,  6,  false,
        {
            int64_t numerator = 0, divisor = 0;
            StackPop(spu.stk, &numerator);
            StackPop(spu.stk, &divisor);

            StackPush(spu.stk, numerator / divisor);

            spu.pc++;
        })

DEF_CMD_(SIN,   7,  false,
        {
            int64_t num = 0;
            StackPop(spu.stk, &num);

            num = (num >= 0) ? sqrt(num) : 0;

            StackPush(spu.stk, num);

            spu.pc++;
        })

DEF_CMD_(COS,   8,  false,
        {
            int64_t num = 0;
            StackPop(spu.stk, &num);

            num = sin(num);

            StackPush(spu.stk, num);

            spu.pc++;
        })

DEF_CMD_(POP,   9,  true,
        {
            int64_t num = 0;
            StackPop(spu.stk, &num);

            num = cos(num);

            StackPush(spu.stk, num);

            spu.pc++;
        })

DEF_CMD_(OUT,   10, false,
        {
            int64_t num_out = 0;
            StackPop(spu.stk, &num_out);

            fprintf(spu.outputFile, "%lld\n", num_out);

            spu.pc++;
        })

DEF_CMD_(IN,    11, false,
        {
            int64_t num_in = 0;
            printf(CYN "enter num:\n" RESET);
            scanf("%lld", &num_in);

            StackPush(spu.stk, num_in);

            spu.pc++;
        })

DEF_CMD_(DUMP,  12, false,
        {
            ProcessorDump(&spu);
            StackDump(spu.stk);

            spu.pc++;
        })

DEF_CMD_(JMP,   13, true,
        {
            int64_t num_arg = 0;
            num_arg = *(nextArg + 1 * SIZE_COMMAND);

            spu.pc = num_arg;
        })

DEF_CMD_(JA,    14, true,
        {
            int64_t num_arg = 0, first_arg = 0, second_arg = 0;
            StackPop(spu.stk, &first_arg);
            StackPop(spu.stk, &second_arg);
            num_arg = *(nextArg + 1 * SIZE_COMMAND);

            if (first_arg > second_arg){
                spu.pc = num_arg;
                break;
            }

            else{
                spu.pc += 2;
                break;
            }
        })

DEF_CMD_(HLT,   15, false,
        {
            RunCommands = 0;
            spu.pc++;
        })

DEF_CMD_(CALL,  16, true,
        {
            int64_t jump_to = 0;
            jump_to = *(nextArg + 1 * SIZE_COMMAND);
            StackPush(spu.returnStack, spu.pc);

            spu.pc = jump_to;
        })

DEF_CMD_(RET,   17, false,
        {
            int64_t num_arg = -1;
            StackPop(spu.returnStack, &num_arg);

            spu.pc = num_arg + 2;
        })

DEF_CMD_(DRAW,  18, false,
        {
            Draw2(&spu);
            spu.pc++;
        })
