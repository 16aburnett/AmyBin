// 32-bit machine language interpreter 
// By Amy Burnett
//========================================================================

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <cstring> //memcpy

//========================================================================

typedef unsigned char byte; 

bool DEBUG = false; 

//========================================================================
// Instructions 
// each instruction is 1-5 bytes
//    iiiiiiii pppppppp pppppppp pppppppp pppppppp
// i - opcode
// p - parameter bits

// note: 64 bit machine has 64 bit pointers (8 byte pointers)
// since our addresses are only 8 bit, this is technically an 8 bit machine

// --- Data Move 

// loadai <value>
// - load given immediate value into 'a' register
// - immediate value should be 4 bytes (enough space for int or float - 32 bit)
//   00000000 00000000 00000000 00000000
byte OPCODE_LOADAI = 0b00000001; 

// loadam <address> 
// - load value that is stored at the given memory address into 'a' register
// - this will load 4 bytes 
byte OPCODE_LOADAM = 0b00000010; 

// loadar <address> 
// - load 4-byte value that is stored at the memory address that is stored in the address register
//   and store it into 'a' register
byte OPCODE_LOADAR = 0b00000011; 

// loadbi <value> 
// - load immediate value into 'b' register 
// - immediate value should be 4 bytes (enough space for int or float - 32 bit)
//   00000000 00000000 00000000 00000000
byte OPCODE_LOADBI = 0b00000100; 

// loadbm <address> 
// - load value at memory address into 'b' register
// - this will load 4 bytes 
byte OPCODE_LOADBM = 0b00000101; 

// loadbr <address> 
// - load 4-byte value that is stored at the memory address that is stored in the address register
//   and store it into 'b' register
byte OPCODE_LOADBR = 0b00000110; 

// loadri <value> 
// - load immediate value into address register 
// - immediate value should be 4 bytes (enough space for int or float - 32 bit)
//   00000000 00000000 00000000 00000000
byte OPCODE_LOADRI = 0b00000111; 

// loadrm <address> 
// - load value at given memory address into address register
// - this will load 4 bytes 
byte OPCODE_LOADRM = 0b00001000; 

// stoream <address>
// - store value from 'a' register into memory at the given address 
// - this will store 4 bytes 
byte OPCODE_STOREAM = 0b00001001; 

// storear <address>
// - store value from 'a' register into memory at the address that is stored in the address register
// - this will store 4 bytes 
byte OPCODE_STOREAR = 0b00001010; 

// storebm <address>
// - store value from 'b' register into memory at the given address 
// - this will store 4 bytes 
byte OPCODE_STOREBM = 0b00001011; 

// storebr <address>
// - store value from 'b' register into memory at the address that is stored in the address register
// - this will store 4 bytes 
byte OPCODE_STOREBR = 0b00001100; 

// storer <address>
// - store value from 'r' register into memory at the given address 
// - this will store 4 bytes 
byte OPCODE_STORERM = 0b00001101; 

// --- arithmetic 

// addint <address>
// - add integer values stored in 'a' and 'b' registers and store at given memory address
byte OPCODE_ADDINT = 0b00100000; 

// addfloat <address> 
// - add floating point values stored in 'a' and 'b' registers and store at given memory address
byte OPCODE_ADDFLOAT = 0b00100001; 

// subint <address>
// - subtracts integer values stored in 'a' and 'b' registers and store at given memory address
byte OPCODE_SUBINT = 0b00100010; 

// subfloat <address>
// - subtracts floating point values stored in 'a' and 'b' registers and store at given memory address
byte OPCODE_SUBFLOAT = 0b00100011; 

// multint <address>
// - multiplies integer values stored in 'a' and 'b' registers and store at given memory address
byte OPCODE_MULTINT = 0b00100100; 

// multfloat <address>
// - multiplies floating point values stored in 'a' and 'b' registers and store at given memory address
byte OPCODE_MULTFLOAT = 0b00100101; 

// divint <address>
// - divides integer values stored in 'a' and 'b' registers and store at given memory address
byte OPCODE_DIVINT = 0b00100110; 

// divfloat <address>
// - divides floating point values stored in 'a' and 'b' registers and store at given memory address
byte OPCODE_DIVFLOAT = 0b00100111; 

// modint <address>
// - mods integer values stored in 'a' and 'b' registers and store at given memory address
byte OPCODE_MODINT = 0b00101000; 

// --- branching 

// cmpint <empty>
// - subtracts 4 byte integers 'b' from 'a' and sets flags based on results 
byte OPCODE_CMPINT = 0b01000000;

// cmpfloat <empty>
// - subtracts 4 byte floats 'b' from 'a' and sets flags based on results 
byte OPCODE_CMPFLOAT = 0b01000001;

// jmp <address>
// - unconditionally changes current instruction to the given address 
byte OPCODE_JMP = 0b01000010;

// jeq <address>
// - jumps to the given address IFF the EQ flag is set 
byte OPCODE_JEQ = 0b01000011;

// jne <address>
// - jumps to the given address IFF the NE flag is set 
byte OPCODE_JNE = 0b01000100;

// jlt <address>
// - jumps to the given address IFF the LT flag is set 
byte OPCODE_JLT = 0b01000101;

// jle <address>
// - jumps to the given address IFF the LE flag is set 
byte OPCODE_JLE = 0b01000110;

// jgt <address>
// - jumps to the given address IFF the GT flag is set 
byte OPCODE_JGT = 0b01000111;

// jge <address>
// - jumps to the given address IFF the GE flag is set 
byte OPCODE_JGE = 0b01001000;

// --- i/o

// printint <address> - 1000 0000 aaaa aaaa
// - prints the decimal integer representation stored at the given memory address
byte OPCODE_PRINTINT = 0b10000000; 

// printchar <address> - 1000 0001 aaaa aaaa
// - prints the character representation stored at the given memory address
byte OPCODE_PRINTCHAR = 0b10000001; 

// printfloat <address> - 1000 0010 aaaa aaaa
// - prints the floating point value stored at the given memory address
byte OPCODE_PRINTFLOAT = 0b10000010; 

// printrint <empty> 
// - prints integer value stored at the address that is stored in the address register
byte OPCODE_PRINTRINT = 0b10000011; 

// printrfloat <empty> 
// - prints float value stored at the address that is stored in the address register
byte OPCODE_PRINTRFLOAT = 0b10000100; 

// --- other

// hlt <empty> - 1111 1111 aaaa aaaa
// - halts the computer 
byte OPCODE_HLT = 0b11111111; 

//========================================================================

void 
printMemory (byte* memory, int memory_size, int bytesPerLine=5)
{
    for (int i = 0; i < memory_size; i+=bytesPerLine)
    {
        // print address 
        printf (
            "0x%x%x%x%x%x%x%x%x | ", 
            (0b11110000000000000000000000000000 & i) >> 28,
            (0b00001111000000000000000000000000 & i) >> 24,
            (0b00000000111100000000000000000000 & i) >> 20,
            (0b00000000000011110000000000000000 & i) >> 16,
            (0b00000000000000001111000000000000 & i) >> 12,
            (0b00000000000000000000111100000000 & i) >>  8,
            (0b00000000000000000000000011110000 & i) >>  4,
            (0b00000000000000000000000000001111 & i) >>  0
        );
        // print binary representation (4 bytes per line)
        for (int j = i; j < i+bytesPerLine; ++j)
        {
            printf (
                "%d%d%d%d%d%d%d%d ",
                (0b10000000 & memory[j]) >> 7,
                (0b01000000 & memory[j]) >> 6,
                (0b00100000 & memory[j]) >> 5,
                (0b00010000 & memory[j]) >> 4,
                (0b00001000 & memory[j]) >> 3,
                (0b00000100 & memory[j]) >> 2,
                (0b00000010 & memory[j]) >> 1,
                (0b00000001 & memory[j]) >> 0
            );
        }
        printf ("| ");
        // print hex representation
        for (int j = i; j < i+bytesPerLine; ++j)
        {
            printf (
                "%x%x ",
                (0b11110000 & memory[j]) >> 4,
                (0b00001111 & memory[j]) >> 0
            );
        }
        printf ("\n");
    }
}

//========================================================================

int 
main()
{

    // allocate memory for the program 
    size_t MEMORY_SIZE_BYTES = 32*4; 
    byte* memory = (byte*) malloc(MEMORY_SIZE_BYTES);

    // define instructions 
    // lttle endian 

    // println (0,n)
    // byte instructions[] = {
    //     // x = 0 
    //     OPCODE_LOADAI,    0x00, 0x00, 0x00, 0x00, // [0x00] 0 -> a 
    //     OPCODE_STOREA,    0x70, 0x00, 0x00, 0x00, // [0x05] a -> x (x is stored as 4 bytes at mem[0x30])
    //     // while x < 10 
    //     OPCODE_LOADAM,    0x70, 0x00, 0x00, 0x00, // [0x0a] x -> a 
    //     OPCODE_LOADBI,    0x00, 0x01, 0x00, 0x00, // [0x0f] 256 -> b (little endian)
    //     OPCODE_CMPINT,    0x00, 0x00, 0x00, 0x00, // [0x14] a < b
    //     OPCODE_JGE,       0x46, 0x00, 0x00, 0x00, // [0x19] if a > b then jmp to end 
    //     // print(x)
    //     OPCODE_PRINTINT,  0x70, 0x00, 0x00, 0x00, // [0x1e] print(x)
    //     // x = x + 1
    //     OPCODE_LOADAM,    0x70, 0x00, 0x00, 0x00, // [0x23] x -> a
    //     OPCODE_LOADBI,    0x01, 0x00, 0x00, 0x00, // [0x28] 1 -> b
    //     OPCODE_ADDINT,    0x70, 0x00, 0x00, 0x00, // [0x2d] a + b -> x
    //     // print('\n')
    //     OPCODE_LOADAI,    (byte) '\n', 0x00, 0x00, 0x00,  // [0x32] '\n' -> a
    //     OPCODE_STOREAM,   0x74, 0x00, 0x00, 0x00, // [0x37] a -> y (y is stored as 4 bytes at mem[0x34])
    //     OPCODE_PRINTCHAR, 0x74, 0x00, 0x00, 0x00, // [0x3c] print(y)
    //     // repeat 
    //     OPCODE_JMP,       0x0a, 0x00, 0x00, 0x00, // [0x41] jmp to start of while loop
    //     OPCODE_HLT,       0x00, 0x00, 0x00, 0x00  // [0x46] halt computer - end of program 
    // };


    // tests floating points
    // float pi = 3.1415f;
    // byte* piBytes = (byte*)&pi;
    // float b = 0.5f;
    // byte* bBytes = (byte*)&b;

    // byte instructions[] = {
    //     OPCODE_LOADAI,     piBytes[0], piBytes[1], piBytes[2], piBytes[3], // [0x00] 3.14f -> a
    //     OPCODE_LOADBI,     bBytes[0], bBytes[1], bBytes[2], bBytes[3], // [0x05] 0.5f -> b
    //     OPCODE_MULTFLOAT,  0x2c, 0x00, 0x00, 0x00, // [0x0a] a * b -> x
    //     OPCODE_PRINTFLOAT, 0x2c, 0x00, 0x00, 0x00, // [0x0f] print(x)
    //     OPCODE_HLT,        0x00, 0x00, 0x00, 0x00  // [0x14] halt
    // };

    // fibonacci 
    // byte instructions[] = {
    //     // initialize start of fibonacci 
    //     OPCODE_LOADAI,     0x00, 0x00, 0x00, 0x00, // [0x00] 0 -> a
    //     OPCODE_STOREAM,    0x70, 0x00, 0x00, 0x00, // [0x05] a -> x
    //     OPCODE_LOADAI,     0x01, 0x00, 0x00, 0x00, // [0x0a] 1 -> a
    //     OPCODE_STOREAM,    0x74, 0x00, 0x00, 0x00, // [0x0f] a -> y
    //     // while y < 1024
    //     OPCODE_LOADAM,     0x70, 0x00, 0x00, 0x00, // [0x14] x -> a
    //     OPCODE_LOADBI,     0x00, 0x04, 0x00, 0x00, // [0x19] 1024 (0x400) -> b
    //     OPCODE_CMPINT,     0x00, 0x00, 0x00, 0x00, // [0x1e] a < b -> update flags
    //     OPCODE_JGE,        0x50, 0x00, 0x00, 0x00, // [0x23] 
    //     // body
    //     OPCODE_PRINTINT,   0x70, 0x00, 0x00, 0x00, // [0x28] print(x)
    //     OPCODE_LOADBI,     '\n', 0x00, 0x00, 0x00, // [0x2d] 'n' -> b
    //     OPCODE_STOREBM,    0x78, 0x00, 0x00, 0x00, // [0x32] b -> newline
    //     OPCODE_PRINTCHAR,  0x78, 0x00, 0x00, 0x00, // [0x37] print(newline)
    //     OPCODE_LOADBM,     0x74, 0x00, 0x00, 0x00, // [0x3c] y -> b
    //     OPCODE_ADDINT,     0x74, 0x00, 0x00, 0x00, // [0x41] a + b -> y
    //     OPCODE_STOREBM,    0x70, 0x00, 0x00, 0x00, // [0x46] b -> x
    //     // repeat
    //     OPCODE_JMP,        0x14, 0x00, 0x00, 0x00, // [0x4b] repeat
    //     OPCODE_HLT,        0x00, 0x00, 0x00, 0x00, // [0x50] halt
    // };

    // print array 
    byte instructions[] = {
        OPCODE_LOADAI,     0x00, 0x00, 0x00, 0x00, // [0x00] 0 -> a
        // while a < 20
        OPCODE_LOADBI,     0x14, 0x00, 0x00, 0x00, // [0x05] 20 -> b
        OPCODE_CMPINT,     0x00, 0x00, 0x00, 0x00, // [0x0a] a < b -> update flags
        OPCODE_JGE,        0x3c, 0x00, 0x00, 0x00, // [0x0f] 
        // body 
        OPCODE_LOADBI,     0x41, 0x00, 0x00, 0x00, // [0x14] arrayAddr -> b
        OPCODE_ADDINT,     0x55, 0x00, 0x00, 0x00, // [0x19] a + b -> newArrayAddr
        OPCODE_LOADRM,     0x55, 0x00, 0x00, 0x00, // [0x1e] [newArrayAddr] -> addrRegister
        OPCODE_PRINTRINT,  0x00, 0x00, 0x00, 0x00, // [0x23] print([addrRegister])
        // increment i by 4 bytes 
        OPCODE_LOADBI,     0x04, 0x00, 0x00, 0x00, // [0x28] 4 -> b
        OPCODE_ADDINT,     0x59, 0x00, 0x00, 0x00, // [0x2d] a + b -> newI
        OPCODE_LOADAM,     0x59, 0x00, 0x00, 0x00, // [0x32] newI -> a
        // repeat 
        OPCODE_JMP,        0x05, 0x00, 0x00, 0x00, // [0x37] repeat
        OPCODE_HLT,        0x00, 0x00, 0x00, 0x00, // [0x3c] halt

        0x0f, 0x00, 0x00, 0x00, // [0x41] 15, arrayAddr
        0x0d, 0x00, 0x00, 0x00, // [0x45] 13
        0x01, 0x00, 0x00, 0x00, // [0x49] 1
        0x02, 0x00, 0x00, 0x00, // [0x4d] 2
        0x06, 0x00, 0x00, 0x00, // [0x51] 6
        0x00, 0x00, 0x00, 0x00, // [0x55] newArrayAddr
        0x00, 0x00, 0x00, 0x00, // [0x59] newI
    };


    // move instructions into memory 
    std::memcpy (memory, instructions, sizeof(instructions));

    // print bytes 
    if (DEBUG)
    {
        printMemory (memory, MEMORY_SIZE_BYTES);
    }

    // execute instructions 
    if (DEBUG) printf ("Running Program\n");

    unsigned int currentInstructionAddress = 0x00;  // 4 byte (32-bit) instruction register
    byte aRegister[] = {0x00, 0x00, 0x00, 0x00};    // 4 byte (32-bit) a register
    byte bRegister[] = {0x00, 0x00, 0x00, 0x00};    // 4 byte (32-bit) b register
    // address register is used to modify addresses and to load/store values at that address
    byte addrRegister[] = {0x00, 0x00, 0x00, 0x00}; // 4 byte (32-bit) addr register
    // XX XX XX XX GT LT EQ NE
    //  0  0  0  0  0  0  0  0
    byte flags = 0b00000000;
    byte FLAG_MASK_GT = 0b00001000; // greater than flag 
    byte FLAG_MASK_LT = 0b00000100; // less than flag 
    byte FLAG_MASK_EQ = 0b00000010; // equal to flag 
    byte FLAG_MASK_NE = 0b00000001; // not equal to flag 
    byte FLAG_CLEAR   = 0b00000000; // cleared flags value 

    while (currentInstructionAddress < MEMORY_SIZE_BYTES)
    {
        byte opcode = memory[currentInstructionAddress];

        if (DEBUG)
        {
            // print address
            printf (
                "0x%x%x%x%x%x%x%x%x | ", 
                (0b11110000000000000000000000000000 & currentInstructionAddress) >> 28,
                (0b00001111000000000000000000000000 & currentInstructionAddress) >> 24,
                (0b00000000111100000000000000000000 & currentInstructionAddress) >> 20,
                (0b00000000000011110000000000000000 & currentInstructionAddress) >> 16,
                (0b00000000000000001111000000000000 & currentInstructionAddress) >> 12,
                (0b00000000000000000000111100000000 & currentInstructionAddress) >>  8,
                (0b00000000000000000000000011110000 & currentInstructionAddress) >>  4,
                (0b00000000000000000000000000001111 & currentInstructionAddress) >>  0
            );
            // print opcode 
            printf (
                "0x%x%x | ", 
                (0b11110000 & opcode) >> 4,
                (0b00001111 & opcode) >> 0
            );
            // print flags 
            printf (
                "flags=%d%d%d%d%d%d%d%d | ",
                (0b10000000 & flags) >> 7,
                (0b01000000 & flags) >> 6,
                (0b00100000 & flags) >> 5,
                (0b00010000 & flags) >> 4,
                (0b00001000 & flags) >> 3,
                (0b00000100 & flags) >> 2,
                (0b00000010 & flags) >> 1,
                (0b00000001 & flags) >> 0
            );
            // print a-reg 
            printf("A: ");
            for (int j = 0; j < 4; ++j)
            {
                printf (
                    "%x%x ",
                    (0b11110000 & aRegister[j]) >> 4,
                    (0b00001111 & aRegister[j]) >> 0
                );
            }
            printf("| B: ");
            // print a-reg 
            for (int j = 0; j < 4; ++j)
            {
                printf (
                    "%x%x ",
                    (0b11110000 & bRegister[j]) >> 4,
                    (0b00001111 & bRegister[j]) >> 0
                );
            }
            printf("\n");
        }

        // --- Data Move 

        // loadai <value> 
        // - load immediate value into 'a' register
        // - immediate value should be 4 bytes (enough space for int or float - 32 bit)
        //   00000000 00000000 00000000 00000000
        if (opcode == OPCODE_LOADAI)
        {
            aRegister[0] = memory[currentInstructionAddress+1];
            aRegister[1] = memory[currentInstructionAddress+2];
            aRegister[2] = memory[currentInstructionAddress+3];
            aRegister[3] = memory[currentInstructionAddress+4];
            // 5 byte instruction
            currentInstructionAddress += 5; 
        }
        
        // loadam <address> 
        // - load value at memory address into 'a' register
        // - this will load 4 bytes 
        else if (opcode == OPCODE_LOADAM)
        {
            int address = *(int*)&memory[currentInstructionAddress+1];
            aRegister[0] = memory[address+0];
            aRegister[1] = memory[address+1];
            aRegister[2] = memory[address+2];
            aRegister[3] = memory[address+3];
            // 5 byte instruction
            currentInstructionAddress += 5; 
        }


        // loadar <address> 
        // - load 4-byte value that is stored at the memory address that is stored in the address register
        //   and store it into 'a' register
        else if (opcode == OPCODE_LOADAR)
        {
            int address = *(int*)addrRegister;
            aRegister[0] = memory[address+0];
            aRegister[1] = memory[address+1];
            aRegister[2] = memory[address+2];
            aRegister[3] = memory[address+3];
            // 5 byte instruction
            currentInstructionAddress += 5; 
        }
        
        // loadbi <value> 
        // - load immediate value into 'b' register   
        // - immediate value should be 4 bytes (enough space for int or float - 32 bit)
        //   00000000 00000000 00000000 00000000  
        else if (opcode == OPCODE_LOADBI)
        {
            bRegister[0] = memory[currentInstructionAddress+1];
            bRegister[1] = memory[currentInstructionAddress+2];
            bRegister[2] = memory[currentInstructionAddress+3];
            bRegister[3] = memory[currentInstructionAddress+4];
            // 5 byte instruction
            currentInstructionAddress += 5; 
        }
        
        // loadbm <address> 
        // - load value at memory address into 'b' register
        // - this will load 4 bytes 
        else if (opcode == OPCODE_LOADBM)
        {
            int address = *(int*)&memory[currentInstructionAddress+1];
            bRegister[0] = memory[address+0];
            bRegister[1] = memory[address+1];
            bRegister[2] = memory[address+2];
            bRegister[3] = memory[address+3];
            // 5 byte instruction
            currentInstructionAddress += 5; 
        }

        // loadbr <address> 
        // - load 4-byte value that is stored at the memory address that is stored in the address register
        //   and store it into 'b' register
        else if (opcode == OPCODE_LOADBR)
        {
            int address = *(int*)addrRegister;
            bRegister[0] = memory[address+0];
            bRegister[1] = memory[address+1];
            bRegister[2] = memory[address+2];
            bRegister[3] = memory[address+3];
            // 5 byte instruction
            currentInstructionAddress += 5; 
        }
        
        // loadri <value> 
        // - load immediate value into address register 
        // - immediate value should be 4 bytes (enough space for int or float - 32 bit)
        else if (opcode == OPCODE_LOADRI)
        {
            addrRegister[0] = memory[currentInstructionAddress+1];
            addrRegister[1] = memory[currentInstructionAddress+2];
            addrRegister[2] = memory[currentInstructionAddress+3];
            addrRegister[3] = memory[currentInstructionAddress+4];
            // 5 byte instruction
            currentInstructionAddress += 5; 
        }

        // loadrm <address> 
        // - load value at given memory address into address register
        // - this will load 4 bytes 
        else if (opcode == OPCODE_LOADRM)
        {
            int address = *(int*)&memory[currentInstructionAddress+1];
            addrRegister[0] = memory[address+0];
            addrRegister[1] = memory[address+1];
            addrRegister[2] = memory[address+2];
            addrRegister[3] = memory[address+3];
            // 5 byte instruction
            currentInstructionAddress += 5; 
        }

        // storea <address>
        // - store value from 'a' register into memory at the given address 
        // - this will store 4 bytes 
        else if (opcode == OPCODE_STOREAM)
        {
            int address = *(int*)&memory[currentInstructionAddress+1];
            memory[address+0] = aRegister[0];
            memory[address+1] = aRegister[1];
            memory[address+2] = aRegister[2];
            memory[address+3] = aRegister[3];
            // 5 byte instruction
            currentInstructionAddress += 5; 
        }

        // storear <address>
        // - store value from 'a' register into memory at the address that is stored in the address register
        // - this will store 4 bytes 
        else if (opcode == OPCODE_STOREAR)
        {
            int address = *(int*)&addrRegister;
            memory[address+0] = aRegister[0];
            memory[address+1] = aRegister[1];
            memory[address+2] = aRegister[2];
            memory[address+3] = aRegister[3];
            // 5 byte instruction
            currentInstructionAddress += 5; 
        }

        // storeb <address>
        // - store value from 'b' register into memory at the given address 
        // - this will store 4 bytes 
        else if (opcode == OPCODE_STOREBM)
        {
            int address = *(int*)&memory[currentInstructionAddress+1];
            memory[address+0] = bRegister[0];
            memory[address+1] = bRegister[1];
            memory[address+2] = bRegister[2];
            memory[address+3] = bRegister[3];
            // 5 byte instruction
            currentInstructionAddress += 5; 
        }

        // storebr <address>
        // - store value from 'b' register into memory at the address that is stored in the address register
        // - this will store 4 bytes 
        else if (opcode == OPCODE_STOREBR)
        {
            int address = *(int*)addrRegister;
            memory[address+0] = bRegister[0];
            memory[address+1] = bRegister[1];
            memory[address+2] = bRegister[2];
            memory[address+3] = bRegister[3];
            // 5 byte instruction
            currentInstructionAddress += 5; 
        }

        // storerm <address>
        // - store value from 'r' register into memory at the given address 
        // - this will store 4 bytes 
        else if (opcode == OPCODE_STORERM)
        {
            int address = *(int*)&memory[currentInstructionAddress+1];
            memory[address+0] = addrRegister[0];
            memory[address+1] = addrRegister[1];
            memory[address+2] = addrRegister[2];
            memory[address+3] = addrRegister[3];
            // 5 byte instruction
            currentInstructionAddress += 5; 
        }

        // --- arithmetic 

        // addint <address> 
        // - add integer values stored in 'a' and 'b' registers and store at given memory address
        else if (opcode == OPCODE_ADDINT)
        {
            // accumulate bytes 
            int a = *(int*)aRegister;
            int b = *(int*)bRegister;
            // perform addition
            int c = a + b; 
            byte* cBytes = (byte*)&c;
            // store result - little endian
            int address = *(int*)&memory[currentInstructionAddress+1];
            memory[address+0] = cBytes[0];
            memory[address+1] = cBytes[1];
            memory[address+2] = cBytes[2];
            memory[address+3] = cBytes[3];
            // 5 byte instruction
            currentInstructionAddress += 5; 
        }

        // addfloat <address> 
        // - add floating point values stored in 'a' and 'b' registers and store at given memory address
        else if (opcode == OPCODE_ADDFLOAT)
        {
            // accumulate bytes 
            float a = *(float*)aRegister;
            float b = *(float*)bRegister;
            // perform addition
            float c = a + b; 
            byte* cBytes = (byte*)&c;
            // store result - little endian
            int address = *(int*)&memory[currentInstructionAddress+1];
            memory[address+0] = cBytes[0];
            memory[address+1] = cBytes[1];
            memory[address+2] = cBytes[2];
            memory[address+3] = cBytes[3];
            // 5 byte instruction
            currentInstructionAddress += 5; 
        }

        // subint <address> 
        // - subtracts integer values stored in 'a' and 'b' registers and store at given memory address
        else if (opcode == OPCODE_SUBINT)
        {
            // accumulate bytes 
            int a = *(int*)aRegister;
            int b = *(int*)bRegister;
            // perform subtraction
            int c = a - b; 
            byte* cBytes = (byte*)&c;
            // store result - little endian
            int address = *(int*)&memory[currentInstructionAddress+1];
            memory[address+0] = cBytes[0];
            memory[address+1] = cBytes[1];
            memory[address+2] = cBytes[2];
            memory[address+3] = cBytes[3];
            // 5 byte instruction
            currentInstructionAddress += 5; 
        }

        // subfloat <address>
        // - subtracts floating point values stored in 'a' and 'b' registers and store at given memory address
        else if (opcode == OPCODE_SUBFLOAT)
        {
            // accumulate bytes 
            float a = *(float*)aRegister;
            float b = *(float*)bRegister;
            // perform subtraction
            float c = a - b; 
            byte* cBytes = (byte*)&c;
            // store result - little endian
            int address = *(int*)&memory[currentInstructionAddress+1];
            memory[address+0] = cBytes[0];
            memory[address+1] = cBytes[1];
            memory[address+2] = cBytes[2];
            memory[address+3] = cBytes[3];
            // 5 byte instruction
            currentInstructionAddress += 5; 
        }

        // multint <address>
        // - multiplies integer values stored in 'a' and 'b' registers and store at given memory address
        else if (opcode == OPCODE_MULTINT)
        {
            // accumulate bytes 
            int a = *(int*)aRegister;
            int b = *(int*)bRegister;
            // perform multiplication
            int c = a * b; 
            byte* cBytes = (byte*)&c;
            // store result - little endian
            int address = *(int*)&memory[currentInstructionAddress+1];
            memory[address+0] = cBytes[0];
            memory[address+1] = cBytes[1];
            memory[address+2] = cBytes[2];
            memory[address+3] = cBytes[3];
            // 5 byte instruction
            currentInstructionAddress += 5; 
        }


        // multfloat <address>
        // - multiplies floating point values stored in 'a' and 'b' registers and store at given memory address
        else if (opcode == OPCODE_MULTFLOAT)
        {
            // accumulate bytes 
            float a = *(float*)aRegister;
            float b = *(float*)bRegister;
            // perform subtraction
            float c = a * b; 
            byte* cBytes = (byte*)&c;
            // store result - little endian
            int address = *(int*)&memory[currentInstructionAddress+1];
            memory[address+0] = cBytes[0];
            memory[address+1] = cBytes[1];
            memory[address+2] = cBytes[2];
            memory[address+3] = cBytes[3];
            // 5 byte instruction
            currentInstructionAddress += 5; 
        }

        // divint <address>
        // - divides integer values stored in 'a' and 'b' registers and store at given memory address
        else if (opcode == OPCODE_DIVINT)
        {
            // accumulate bytes 
            int a = *(int*)aRegister;
            int b = *(int*)bRegister;
            // perform multiplication
            int c = a / b; 
            byte* cBytes = (byte*)&c;
            // store result - little endian
            int address = *(int*)&memory[currentInstructionAddress+1];
            memory[address+0] = cBytes[0];
            memory[address+1] = cBytes[1];
            memory[address+2] = cBytes[2];
            memory[address+3] = cBytes[3];
            // 5 byte instruction
            currentInstructionAddress += 5; 
        }

        // divfloat <address>
        // - divides floating point values stored in 'a' and 'b' registers and store at given memory address
        else if (opcode == OPCODE_DIVFLOAT)
        {
            // accumulate bytes 
            float a = *(float*)aRegister;
            float b = *(float*)bRegister;
            // perform subtraction
            float c = a / b; 
            byte* cBytes = (byte*)&c;
            // store result - little endian
            int address = *(int*)&memory[currentInstructionAddress+1];
            memory[address+0] = cBytes[0];
            memory[address+1] = cBytes[1];
            memory[address+2] = cBytes[2];
            memory[address+3] = cBytes[3];
            // 5 byte instruction
            currentInstructionAddress += 5; 
        }

        // modint <address>
        // - mods integer values stored in 'a' and 'b' registers and store at given memory address
        else if (opcode == OPCODE_MODINT)
        {
            // accumulate bytes 
            int a = *(int*)aRegister;
            int b = *(int*)bRegister;
            // perform multiplication
            int c = a % b; 
            byte* cBytes = (byte*)&c;
            // store result - little endian
            int address = *(int*)&memory[currentInstructionAddress+1];
            memory[address+0] = cBytes[0];
            memory[address+1] = cBytes[1];
            memory[address+2] = cBytes[2];
            memory[address+3] = cBytes[3];
            // 5 byte instruction
            currentInstructionAddress += 5; 
        }

        // --- branching 

        // cmpint <empty>
        // - subtracts integer 'b' from 'a' and sets flags based on results 
        else if (opcode == OPCODE_CMPINT)
        {
            // accumulate bytes 
            int a = *(int*)aRegister;
            int b = *(int*)bRegister;
            // clear flags 
            // set flags 
            flags = FLAG_CLEAR;
            if (a < b) flags |= FLAG_MASK_LT;
            if (a == b) flags |= FLAG_MASK_EQ;
            else flags |= FLAG_MASK_NE; 
            if (a > b) flags |= FLAG_MASK_GT;
            // 5 byte instruction
            currentInstructionAddress += 5; 
        }
        
        // jmp <address>
        // - unconditionally changes current instruction to the given address 
        else if (opcode == OPCODE_JMP)
        {
            currentInstructionAddress = *(int*)&memory[currentInstructionAddress+1]; 
            continue;
        }

        // jeq <address>
        // - jumps to the given address IFF the EQ flag is set 
        else if (opcode == OPCODE_JEQ)
        {
            if (flags & FLAG_MASK_EQ)
            {
                currentInstructionAddress = *(int*)&memory[currentInstructionAddress+1]; 
                continue;  
            }
            // 5 byte instruction
            currentInstructionAddress += 5; 
        }

        // jne <address>
        // - jumps to the given address IFF the NE flag is set 
        else if (opcode == OPCODE_JNE)
        {
            if (flags & FLAG_MASK_NE)
            {
                currentInstructionAddress = *(int*)&memory[currentInstructionAddress+1]; 
                continue;
            }
            // 5 byte instruction
            currentInstructionAddress += 5; 
        }

        // jlt <address>
        // - jumps to the given address IFF the LT flag is set 
        else if (opcode == OPCODE_JLT)
        {
            if (flags & FLAG_MASK_LT)
            {
                currentInstructionAddress = *(int*)&memory[currentInstructionAddress+1]; 
                continue;
            }
            // 5 byte instruction
            currentInstructionAddress += 5; 
        }

        // jle <address>
        // - jumps to the given address IFF the LE flag is set 
        else if (opcode == OPCODE_JLE)
        {
            if (flags & (FLAG_MASK_LT | FLAG_MASK_EQ))
            {
                currentInstructionAddress = *(int*)&memory[currentInstructionAddress+1]; 
                continue;
            }
            // 5 byte instruction
            currentInstructionAddress += 5; 
        }

        // jgt <address>
        // - jumps to the given address IFF the GT flag is set 
        else if (opcode == OPCODE_JGT)
        {
            if (flags & FLAG_MASK_GT)
            {
                currentInstructionAddress = *(int*)&memory[currentInstructionAddress+1]; 
                continue;
            }
            // 5 byte instruction
            currentInstructionAddress += 5; 
        }

        // jge <address>
        // - jumps to the given address IFF the GE flag is set 
        else if (opcode == OPCODE_JGE)
        {
            if (flags & (FLAG_MASK_GT | FLAG_MASK_EQ))
            {
                currentInstructionAddress = *(int*)&memory[currentInstructionAddress+1]; 
                continue;
            }
            // 5 byte instruction
            currentInstructionAddress += 5; 
        }

        // --- i/o

        // printint <address>
        // - prints the decimal integer representation stored at the given memory address
        else if (opcode == OPCODE_PRINTINT)
        {
            int address = *(int*)&memory[currentInstructionAddress+1];
            int value = *(int*)(memory+address);
            printf ("%d", value);
            // 5 byte instruction
            currentInstructionAddress += 5; 
        }

        // printchar <address>
        // - prints the character representation stored at the given memory address
        else if (opcode == OPCODE_PRINTCHAR)
        {
            int address = *(int*)&memory[currentInstructionAddress+1];
            // cast to an int because by default it is 4 bytes 
            printf ("%c", *(char*)(memory+address));
            // 5 byte instruction
            currentInstructionAddress += 5; 
        }

        // printfloat <address>
        // - prints the floating point value stored at the given memory address
        else if (opcode == OPCODE_PRINTFLOAT)
        {
            int address = *(int*)&memory[currentInstructionAddress+1];
            printf ("%f", *(float*)(memory+address));
            // 5 byte instruction
            currentInstructionAddress += 5; 
        }

        // printrint <address> 
        // - prints integer value stored at the address that is stored in the address register
        else if (opcode == OPCODE_PRINTRINT)
        {
            // interpret 4 bytes from addrRegister as an integer 
            int address = *(int*)&addrRegister;
            printf ("%d", *(int*)(memory+address));
            // 5 byte instruction
            currentInstructionAddress += 5; 
        }

        // printrfloat <address> 
        // - prints float value stored at the address that is stored in the address register
        else if (opcode == OPCODE_PRINTRFLOAT)
        {
            // interpret 4 bytes from addrRegister as an integer 
            int address = *(int*)&addrRegister;
            printf ("%f", *(float*)(memory+address));
            // 5 byte instruction
            currentInstructionAddress += 5; 
        }

        // --- other

        // hlt <empty> - 1111 1111 aaaa aaaa
        // - halts the computer 
        else if (opcode == OPCODE_HLT)
        {
            if (DEBUG) printf ("Halting Computer\n");
            break; 
        }

        // unknown instruction
        else
        {
            printf ("Invalid opcode %x%x\n", 
                (0b11110000 & opcode) >> 4,
                (0b00001111 & opcode) >> 0
            );
            break; 
        }

    }

    if (DEBUG) printf ("Program Finished\n");

    // print bytes 
    if (DEBUG)
    {
        printMemory (memory, MEMORY_SIZE_BYTES);
    }

}

//========================================================================
