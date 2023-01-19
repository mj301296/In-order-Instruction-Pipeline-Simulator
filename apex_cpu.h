/*
 * apex_cpu.h
 * Contains APEX cpu pipeline declarations
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#ifndef _APEX_CPU_H_
#define _APEX_CPU_H_

#include "apex_macros.h"

/* Format of an APEX instruction  */
typedef struct APEX_Instruction
{
    char opcode_str[128];
    int opcode;
    int rd;
    int rs1;
    int rs2;
    int rs3;
    int imm;
    int number; /*Instruction number to track in queue*/
} APEX_Instruction;

/* Model of CPU stage latch */
typedef struct CPU_Stage
{
    int pc;
    char opcode_str[128];
    int opcode;
    int rs1;
    int rs2;
    int rs3;
    int rd;
    int imm;
    int number; /*Instruction number to track in queue*/
    int rs1_value;
    int rs2_value;
    int rs3_value;
    int result_buffer;
    int memory_address;
    int stall; /* 0 : STAGE IS FREE */
               /* 1 : STAGE IS BUSY */
               /* 2 : OUTPUT IS READY */
    int cycle; /* To track number of cycles in multiplier and load/store FU*/
    int has_insn;
} CPU_Stage;

/* Model of APEX CPU */
typedef struct APEX_CPU
{
    int pc;                            /* Current program counter */
    int clock;                         /* Clock cycles elapsed */
    int insn_completed;                /* Instructions retired */
    int regs[REG_FILE_SIZE];           /* Integer register file */
    int state_regs[REG_FILE_SIZE];     /* State of the registers*/
    int code_memory_size;              /* Number of instruction in the input file */
    APEX_Instruction *code_memory;     /* Code Memory */
    int data_memory[DATA_MEMORY_SIZE]; /* Data Memory */
    int single_step;                   /* Wait for user input after every cycle */
    int zero_flag;                     /* {TRUE, FALSE} Used by BZ and BNZ to branch */
    int fetch_from_next_cycle;
    int command;
    int command_2;
    int instruction_queue[QUEUE_SIZE]; /* Queue to hold instruction*/
    int Rear;
    int Front;

    /* Pipeline stages */
    CPU_Stage fetch;
    CPU_Stage decode;
    CPU_Stage integer;
    CPU_Stage multiplier;
    CPU_Stage load_store;
    CPU_Stage writeback;
} APEX_CPU;

APEX_Instruction *create_code_memory(const char *filename, int *size);
APEX_CPU *APEX_cpu_init(const char *filename);
int APEX_cpu_simulator(const char *command);
void APEX_cpu_run(APEX_CPU *cpu);
void APEX_cpu_stop(APEX_CPU *cpu);
#endif
