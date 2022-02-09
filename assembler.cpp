// 32-bit machine language interpreter 
// that uses a RISC-V-like format 
// By Amy Burnett
//========================================================================

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <cstring>   //memcpy

//========================================================================

typedef unsigned char byte; 

bool DEBUG = false; 
// 1MB by default 
size_t MEMORY_SIZE_BYTES = 1000000; 

// makes sure each opcode is unique
// opcode 0 is not defined 
// which will halt the program with an error 
byte opcode_counter = 0b00000001;

// LUI dest, imm        - loads upper immediate 16 bits into given register
// XXXXXXXX dddd0000 iiiiiiii iiiiiiii
byte OPCODE_LUI = opcode_counter++;
// LLI dest, imm        - loads lower immediate 16 bits into given register
// XXXXXXXX dddd0000 iiiiiiii iiiiiiii
byte OPCODE_LLI = opcode_counter++;
// LB dest, offset(src) - load byte 
// XXXXXXXX ddddssss oooooooo oooooooo
byte OPCODE_LB = opcode_counter++;
// LH dest, offset(src) - load half (2 bytes)
// XXXXXXXX ddddssss oooooooo oooooooo
byte OPCODE_LH = opcode_counter++;
// LW dest, offset(src) - load word (4 bytes)
// XXXXXXXX ddddssss oooooooo oooooooo
byte OPCODE_LW = opcode_counter++;
// SB offset(dest), src - store byte
// XXXXXXXX ddddssss oooooooo oooooooo
byte OPCODE_SB = opcode_counter++;
// SH offset(dest), src - store half (2 bytes)
// XXXXXXXX ddddssss oooooooo oooooooo
byte OPCODE_SH = opcode_counter++;
// SW offset(dest), src - store word (4 bytes)
// XXXXXXXX ddddssss oooooooo oooooooo
byte OPCODE_SW = opcode_counter++;

// arithmetic instructions
// ADD dest, src1, src2 - integer addition
// XXXXXXXX ddddssss ssss0000 00000000
byte OPCODE_ADD = opcode_counter++;
// SUB dest, src1, src2 - integer subtraction
// XXXXXXXX ddddssss ssss0000 00000000
byte OPCODE_SUB = opcode_counter++;
// MUL dest, src1, src2 - integer multiplication
// XXXXXXXX ddddssss ssss0000 00000000
byte OPCODE_MUL = opcode_counter++;
// DIV dest, src1, src2 - integer division
// XXXXXXXX ddddssss ssss0000 00000000
byte OPCODE_DIV = opcode_counter++;
// MOD dest, src1, src2 - integer division remainder
// XXXXXXXX ddddssss ssss0000 00000000
byte OPCODE_MOD = opcode_counter++;
// SLL dest, src1, src2 - shift left logical
// XXXXXXXX ddddssss ssss0000 00000000
byte OPCODE_SLL = opcode_counter++;
// SRL dest, src1, src2 - shift right logical
// XXXXXXXX ddddssss ssss0000 00000000
byte OPCODE_SRL = opcode_counter++;
// SRA dest, src1, src2 - shift right arithmetic
// XXXXXXXX ddddssss ssss0000 00000000
byte OPCODE_SRA = opcode_counter++;
// OR  dest, src1, src2 - bitwise or
// XXXXXXXX ddddssss ssss0000 00000000
byte OPCODE_OR  = opcode_counter++;
// AND dest, src1, src2 - bitwise and
// XXXXXXXX ddddssss ssss0000 00000000
byte OPCODE_AND = opcode_counter++;
// XOR dest, src1, src2 - bitwise xor 
// XXXXXXXX ddddssss ssss0000 00000000
byte OPCODE_XOR = opcode_counter++;

// immediate arithmetic instructions
// immediate values are 16-bit signed
// ADDI dest, src1, imm - integer addition with immediate
// - can be used to load immediate into register 
// - that's why there is no load immediate 
// - ADDI r0, rzero, 42 : r0 <- 0 + 42
// XXXXXXXX ddddssss iiiiiiii iiiiiiii
byte OPCODE_ADDI = opcode_counter++;
// SUBI dest, src1, imm - integer subtraction with immediate
// XXXXXXXX ddddssss iiiiiiii iiiiiiii
byte OPCODE_SUBI = opcode_counter++;
// MULI dest, src1, src2 - integer multiplication with immediate
// XXXXXXXX ddddssss iiiiiiii iiiiiiii
byte OPCODE_MULI = opcode_counter++;
// DIVI dest, src1, src2 - integer division with immediate
// XXXXXXXX ddddssss iiiiiiii iiiiiiii
byte OPCODE_DIVI = opcode_counter++;
// MODI dest, src1, src2 - integer division remainder with immediate
// XXXXXXXX ddddssss iiiiiiii iiiiiiii
byte OPCODE_MODI = opcode_counter++;
// SLLI dest, src1, imm - shift left logical with immediate
// XXXXXXXX ddddssss iiiiiiii iiiiiiii
byte OPCODE_SLLI = opcode_counter++;
// SRLI dest, src1, imm - shift right logical with immediate
// XXXXXXXX ddddssss iiiiiiii iiiiiiii
byte OPCODE_SRLI = opcode_counter++;
// SRAI dest, src1, imm - shift right arithmetic with immediate
// XXXXXXXX ddddssss iiiiiiii iiiiiiii
byte OPCODE_SRAI = opcode_counter++;
// ORI  dest, src1, imm - bitwise or with immediate
// XXXXXXXX ddddssss iiiiiiii iiiiiiii
byte OPCODE_ORI  = opcode_counter++;
// ANDI dest, src1, imm - bitwise and with immediate
// XXXXXXXX ddddssss iiiiiiii iiiiiiii
byte OPCODE_ANDI = opcode_counter++;
// XORI dest, src1, imm - bitwise xor  with immediate
// XXXXXXXX ddddssss iiiiiiii iiiiiiii
byte OPCODE_XORI = opcode_counter++;

// branching
// BEQ src1, src2, addr - if src1 == src2 then pc <- addr
// XXXXXXXX ssssssss aaaa0000 00000000
byte OPCODE_BEQ = opcode_counter++;
// BNE src1, src2, addr - if src1 != src2 then pc <- addr
// XXXXXXXX ssssssss aaaa0000 00000000
byte OPCODE_BNE = opcode_counter++;
// BLT src1, src2, addr - if src1 <  src2 then pc <- addr
// XXXXXXXX ssssssss aaaa0000 00000000
byte OPCODE_BLT = opcode_counter++;
// BLE src1, src2, addr - if src1 <= src2 then pc <- addr
// XXXXXXXX ssssssss aaaa0000 00000000
byte OPCODE_BLE = opcode_counter++;
// BGT src1, src2, addr - if src1 >  src2 then pc <- addr
// XXXXXXXX ssssssss aaaa0000 00000000
byte OPCODE_BGT = opcode_counter++;
// BGE src1, src2, addr - if src1 >= src2 then pc <- addr
// XXXXXXXX ssssssss aaaa0000 00000000
byte OPCODE_BGE = opcode_counter++;
// JMP addr - pc <- addr
// XXXXXXXX aaaa0000 00000000 00000000
byte OPCODE_JMP = opcode_counter++;

// function instructions
// CALL addr
// 1. pushes return address on to the stack
// 2. changes pc to addr
// base pointer should be pushed on the stack by the callee
// push bp
// mov bp, sp
// Caller's actions
// 1. push caller saved registers
// 2. push args in reverse order (callee can access with arg0 = [bp+8], arg1 = [bp+12])
// 3. call function
// Call's actions
// 1. push return addr
// 2. pc <- addr 
// Callee's actions 
// 1. push caller's bp 
// 2. align our frame's bp and sp (mov bp, sp)
// 3. allocate space for local vars (sub sp, sp, <#bytes>)
//    local vars can be access with bp - 0, bp - 4, bp - 8, etc
// 4. push callee saved registers onto stack 
//    these need to be restored because caller 
//    expects these values to be unchanged. 
// XXXXXXXX aaaa0000 00000000 00000000
byte OPCODE_CALL = opcode_counter++;
// RET - pc <- [bp]
// changes the current pc to the return address pointed to by bp
// Callee's actions before returning
// 1. store any return value in ra (return value register)
// 2. restore callee-saved registers 
// 3. pop local vars off of stack (mov sp, bp) (sp <- bp)
// 4. restore caller's bp (pop bp)
// Return's actions 
// 1. pops return address off of stack and stores in pc (pop pc) 
// Caller's actions after returning 
// 1. pop any arguments that were pushed onto the stack (add sp, sp, <#bytes>)
// 2. pop any caller saved registers back into their respective registers (pop r#)
// XXXXXXXX 00000000 00000000 00000000
byte OPCODE_RET = opcode_counter++;
// PUSH src - sp -= 4 ; [sp] <- src
// 1. decrements sp by 4 (bytes)
// 2. places src onto stack at [sp]
// XXXXXXXX ssss0000 00000000 00000000
byte OPCODE_PUSH = opcode_counter++;
// POP dest - dest <- [sp] ; sp += 4
// 1. moves [sp] into dest 
// 2. increments sp by 4 (bytes)
// XXXXXXXX dddd0000 00000000 00000000
byte OPCODE_POP = opcode_counter++;

// other instructions
// NOP - no operation
// XXXXXXXX 000000000 00000000 00000000
byte OPCODE_NOP = opcode_counter++;
// HLT - halts the computer
// XXXXXXXX 000000000 00000000 00000000
byte OPCODE_HLT = opcode_counter++;
// GETCHAR - reads (from stdin) a char (1-byte) and stores it in the 
// given register
// XXXXXXXX dddd00000 00000000 00000000
byte OPCODE_GETCHAR = opcode_counter++;
// PUTCHAR - outputs (to stdout) a char (1-byte) from the given register
// XXXXXXXX ssss00000 00000000 00000000
byte OPCODE_PUTCHAR = opcode_counter++;



//========================================================================



//========================================================================
