/*
 * apex_cpu.c
 * Contains APEX cpu pipeline implementation
 *
 * Author:
 * Copyright (c) 2020, Gaurav Kothari (gkothar1@binghamton.edu)
 * State University of New York at Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "apex_cpu.h"
#include "apex_macros.h"

// static int stall = 0;

/* Converts the PC(4000 series) into array index for code memory
 *
 * Note: You are not supposed to edit this function
 */
static int
get_code_memory_index_from_pc(const int pc)
{
    return (pc - 4000) / 4;
}

static void
print_instruction(const CPU_Stage *stage)
{
    switch (stage->opcode)
    {
    case OPCODE_ADD:
    {
        printf("%s,R%d,R%d,R%d  I%d", stage->opcode_str, stage->rd, stage->rs1, stage->rs2, stage->number);
        break;
    }
    case OPCODE_ADDL:
    {
        printf("%s,R%d,R%d,#%d  I%d", stage->opcode_str, stage->rd, stage->rs1, stage->imm, stage->number);
        break;
    }
    case OPCODE_SUB:
    {
        printf("%s,R%d,R%d,R%d I%d", stage->opcode_str, stage->rd, stage->rs1, stage->rs2, stage->number);
        break;
    }
    case OPCODE_SUBL:
    {
        printf("%s,R%d,R%d,#%d  I%d", stage->opcode_str, stage->rd, stage->rs1, stage->imm, stage->number);
        break;
    }
    case OPCODE_MUL:
    {
        printf("%s,R%d,R%d,R%d  I%d", stage->opcode_str, stage->rd, stage->rs1, stage->rs2, stage->number);
        break;
    }
    case OPCODE_DIV:
    {
        printf("%s,R%d,R%d,R%d  I%d", stage->opcode_str, stage->rd, stage->rs1, stage->rs2, stage->number);
        break;
    }
    case OPCODE_AND:
    {
        printf("%s,R%d,R%d,R%d  I%d", stage->opcode_str, stage->rd, stage->rs1, stage->rs2, stage->number);
        break;
    }
    case OPCODE_OR:
    {
        printf("%s,R%d,R%d,R%d  I%d", stage->opcode_str, stage->rd, stage->rs1, stage->rs2, stage->number);
        break;
    }
    case OPCODE_XOR:
    {
        printf("%s,R%d,R%d,R%d  I%d", stage->opcode_str, stage->rd, stage->rs1,
               stage->rs2, stage->number);
        break;
    }

    case OPCODE_CMP:
    {
        printf("%s,R%d,R%d  I%d", stage->opcode_str, stage->rs1, stage->rs2, stage->number);
        break;
    }

    case OPCODE_MOVC:
    {
        printf("%s,R%d,#%d  I%d", stage->opcode_str, stage->rd, stage->imm, stage->number);
        break;
    }

    case OPCODE_LOAD:
    {
        printf("%s,R%d,R%d,#%d  I%d", stage->opcode_str, stage->rd, stage->rs1,
               stage->imm, stage->number);
        break;
    }

    case OPCODE_STORE:
    {
        printf("%s,R%d,R%d,#%d I%d", stage->opcode_str, stage->rs1, stage->rs2,
               stage->imm, stage->number);
        break;
    }

    case OPCODE_LDR:
    {
        printf("%s,R%d,R%d,R%d  I%d", stage->opcode_str, stage->rd, stage->rs1,
               stage->rs2, stage->number);
        break;
    }

    case OPCODE_STR:
    {
        printf("%s,R%d,R%d,R%d I%d", stage->opcode_str, stage->rs3, stage->rs1,
               stage->rs2, stage->number);
        break;
    }

    case OPCODE_BZ:
    {
        printf("%s,#%d I%d", stage->opcode_str, stage->imm, stage->number);
        break;
    }
    case OPCODE_BNZ:
    {
        printf("%s,#%d  I%d", stage->opcode_str, stage->imm, stage->number);
        break;
    }

    case OPCODE_NOP:
    {
        printf("%s I%d", stage->opcode_str, stage->number);
        break;
    }

    case OPCODE_HALT:
    {
        printf("%s I%d", stage->opcode_str, stage->number);
        break;
    }
    }
}

/* Debug function which prints the CPU stage content
 *
 * Note: You can edit this function to print in more detail
 */
static void
print_stage_content(const char *name, const CPU_Stage *stage)
{
    printf("%-15s: pc(%d) ", name, stage->pc);
    print_instruction(stage);
    printf("\n");
}

/* Debug function which prints the register file
 *
 * Note: You are not supposed to edit this function
 */
static void
print_reg_file(const APEX_CPU *cpu)
{
    int i;

    printf("----------\n%s\n----------\n", "Registers:");

    for (int i = 0; i < REG_FILE_SIZE / 2; ++i)
    {
        printf("R%-3d[%-3d] ", i, cpu->regs[i]);
    }

    printf("\n");

    for (i = (REG_FILE_SIZE / 2); i < REG_FILE_SIZE; ++i)
    {
        printf("R%-3d[%-3d] ", i, cpu->regs[i]);
    }

    printf("\n");
}

static void
print_memory_file(const APEX_CPU *cpu)
{
    int i = 0;
    int start = 0;
    int end = DATA_MEMORY_SIZE;

    printf("----------\n%s\n----------\n", "Data Memory:");
    while (end != 0)
    {
        for (i = start; i < start + 8; ++i)
        {
            if (cpu->data_memory[i] != 0)
            {
                printf("MEM[%-2d]=%-2d ", i, cpu->data_memory[i]);
            }
        }

        //   printf("\n");
        end = end - 8;
        start = start + 8;
    }
    printf("\n");
}

static void enqueue(APEX_CPU *cpu, int insert_item)
{
    if (cpu->Rear == QUEUE_SIZE - 1)
        printf("Overflow \n");
    else
    {
        if (cpu->Front == -1)
            cpu->Front = 0;
        cpu->Rear = cpu->Rear + 1;
        cpu->instruction_queue[cpu->Rear] = insert_item;
    }
}

static void dequeue(APEX_CPU *cpu)
{
    if (cpu->Front == -1 || cpu->Front > cpu->Rear)
    {
        printf("Underflow \n");
        return;
    }
    else
    {
        if (ENABLE_DEBUG_MESSAGES)
        {
            printf("Element deleted from the Queue: %d\n", cpu->instruction_queue[cpu->Front]);
        }
        cpu->Front = cpu->Front + 1;
    }
}

static void show(APEX_CPU *cpu)
{

    if (cpu->Front == -1)
        printf("Empty Queue \n");
    else
    {
        printf("Queue: \n");
        for (int i = cpu->Front; i <= cpu->Rear; i++)
            printf("%d ", cpu->instruction_queue[i]);
        printf("\n");
    }
}

/*
 * Fetch Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_fetch(APEX_CPU *cpu)
{
    APEX_Instruction *current_ins;
    if (cpu->fetch.has_insn)
    {

        /* This fetches new branch target instruction from next cycle */
        if (cpu->fetch_from_next_cycle == TRUE)
        {
            cpu->fetch_from_next_cycle = FALSE;

            /* Skip this cycle*/
            return;
        }
        if (cpu->decode.stall == 0)
        {
            /* Store current PC in fetch latch */
            cpu->fetch.pc = cpu->pc;

            /* Index into code memory using this pc and copy all instruction fields
             * into fetch latch  */
            current_ins = &cpu->code_memory[get_code_memory_index_from_pc(cpu->pc)];
            strcpy(cpu->fetch.opcode_str, current_ins->opcode_str);
            cpu->fetch.opcode = current_ins->opcode;
            cpu->fetch.rd = current_ins->rd;
            cpu->fetch.rs1 = current_ins->rs1;
            cpu->fetch.rs2 = current_ins->rs2;
            cpu->fetch.rs3 = current_ins->rs3;
            cpu->fetch.imm = current_ins->imm;
            cpu->fetch.number = current_ins->number;
            /* Update PC for next instruction */
            cpu->pc += 4;
            /* Copy data from fetch latch to decode latch*/
            cpu->decode = cpu->fetch;

            if ((ENABLE_DEBUG_MESSAGES) || (cpu->command == DISPLAY) || (cpu->command == SINGLE_STEP))
            {
                print_stage_content("Fetch", &cpu->fetch);
            }

            /* Stop fetching new instructions if HALT is fetched */
            if (cpu->fetch.opcode == OPCODE_HALT)
            {
                cpu->fetch.has_insn = FALSE;
            }
        }
    }
}

/*
 * Decode Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_decode(APEX_CPU *cpu)
{
    if (cpu->decode.has_insn)
    {
        if (cpu->decode.stall)
        {
            if ((cpu->decode.opcode == OPCODE_STR) || (cpu->decode.opcode == OPCODE_LDR) || (cpu->decode.opcode == OPCODE_LOAD) || (cpu->decode.opcode == OPCODE_STORE))
            {
                /*Incase if it's the first time the instruction has entered incase of stall then first decode the instruction*/
                if (cpu->load_store.stall == 0)
                {
                    cpu->decode.stall = 0;
                    goto ALLOW_DECODE;
                }
                else
                {
                    if (((cpu->decode.opcode == OPCODE_LOAD) || (cpu->decode.opcode == OPCODE_LDR)) && (cpu->state_regs[cpu->decode.rd] == 1))
                    {
                        cpu->state_regs[cpu->decode.rd] = 0;
                    }
                    cpu->fetch_from_next_cycle = TRUE;
                }
            }
            else if (cpu->decode.opcode == OPCODE_MUL)
            {
                if (cpu->multiplier.stall == 0)
                {
                    cpu->decode.stall = 0;
                    goto ALLOW_DECODE;
                }
                else
                {
                    if (cpu->state_regs[cpu->decode.rd] == 1)
                    {
                        cpu->state_regs[cpu->decode.rd] = 0;
                    }
                    cpu->fetch_from_next_cycle = TRUE;
                }
            }
            else
            {
                if (cpu->integer.stall == 0)
                {
                    cpu->decode.stall = 0;
                    goto ALLOW_DECODE;
                }
                else
                {
                    if (cpu->state_regs[cpu->decode.rd] == 1)
                    {
                        cpu->state_regs[cpu->decode.rd] = 0;
                    }
                    cpu->fetch_from_next_cycle = TRUE;
                }
            }
        }
        else
        {
        ALLOW_DECODE:;
            /*MJXX simple scoreboarding logic*/
            /*set the status of registers which are currently being used as 1*/
            if (cpu->state_regs[cpu->decode.rd] == 1 || cpu->state_regs[cpu->decode.rs1] == 1 || cpu->state_regs[cpu->decode.rs2] == 1 || cpu->state_regs[cpu->decode.rs3] == 1)
            {
                if (ENABLE_DEBUG_MESSAGES)
                {
                    printf("\nMJXX inside decode:operation has been stalled\n");
                }
                if ((ENABLE_DEBUG_MESSAGES) || (cpu->command == DISPLAY) || (cpu->command == SINGLE_STEP))
                {
                    print_stage_content("Decode/RF", &cpu->decode);
                }
                cpu->fetch_from_next_cycle = TRUE;
            }
            else
            {
                /* MJXX simple scoreboarding logic*/
                /* Set the destination register state indicator till the instruction execution is completed*/
                if ((cpu->decode.opcode != OPCODE_STORE) && (cpu->decode.opcode != OPCODE_STR) && (cpu->decode.opcode != OPCODE_CMP) && (cpu->decode.opcode != OPCODE_NOP) && (cpu->decode.opcode != OPCODE_BZ) && (cpu->decode.opcode != OPCODE_BNZ))
                {
                    cpu->state_regs[cpu->decode.rd] = 1;
                }
                /* Read operands from register file based on the instruction type */
                switch (cpu->decode.opcode)
                {
                case OPCODE_ADD:
                {
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                    break;
                }
                case OPCODE_ADDL:
                {
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    break;
                }
                case OPCODE_SUB:
                {
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                    break;
                }
                case OPCODE_SUBL:
                {
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    break;
                }
                case OPCODE_MUL:
                {
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                    break;
                }
                case OPCODE_DIV:
                {
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                    break;
                }
                case OPCODE_AND:
                {
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                    break;
                }
                case OPCODE_OR:
                {
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                    break;
                }
                case OPCODE_XOR:
                {
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                    break;
                }
                case OPCODE_CMP:
                {
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                    break;
                }

                case OPCODE_LOAD:
                {
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    break;
                }
                case OPCODE_STORE:
                {
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                    break;
                }
                case OPCODE_LDR:
                {
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                    break;
                }
                case OPCODE_STR:
                {
                    cpu->decode.rs3_value = cpu->regs[cpu->decode.rs3];
                    cpu->decode.rs1_value = cpu->regs[cpu->decode.rs1];
                    cpu->decode.rs2_value = cpu->regs[cpu->decode.rs2];
                    break;
                }

                case OPCODE_MOVC:
                {
                    /* MOVC doesn't have register operands */
                    break;
                }
                }

                /* Copy data from decode latch to execute latch*/
                /* Incase FU unit is busy stall the instructions else push instruction into queue*/
                if ((cpu->decode.opcode == OPCODE_STR) || (cpu->decode.opcode == OPCODE_LDR) || (cpu->decode.opcode == OPCODE_LOAD) || (cpu->decode.opcode == OPCODE_STORE))
                {
                    if (cpu->load_store.stall == 0)
                    {
                        cpu->decode.stall = 0;
                        cpu->load_store = cpu->decode;
                    }
                    else
                    {
                        if (((cpu->decode.opcode == OPCODE_LOAD) || (cpu->decode.opcode == OPCODE_LDR)) && (cpu->state_regs[cpu->decode.rd] == 1))
                        {
                            cpu->state_regs[cpu->decode.rd] = 0;
                        }
                        //        cpu->fetch_from_next_cycle = TRUE;
                        cpu->decode.stall = 1;
                        goto STALL;
                    }
                }
                else if (cpu->decode.opcode == OPCODE_MUL)
                {
                    if (cpu->multiplier.stall == 0)
                    {
                        cpu->decode.stall = 0;
                        cpu->multiplier = cpu->decode;
                    }
                    else
                    {
                        if (cpu->state_regs[cpu->decode.rd] == 1)
                        {
                            cpu->state_regs[cpu->decode.rd] = 0;
                        }
                        cpu->decode.stall = 1;
                        goto STALL;
                    }
                }
                else
                {
                    if (cpu->integer.stall == 0)
                    {
                        cpu->decode.stall = 0;
                        cpu->integer = cpu->decode;
                    }
                    else
                    {
                        cpu->decode.stall = 1;
                        goto STALL;
                    }
                }
                enqueue(cpu, cpu->decode.number);
                if (ENABLE_DEBUG_MESSAGES)
                {
                    printf("MJXX: value to be added:%d\n", cpu->decode.number);
                    show(cpu);
                }
                cpu->decode.has_insn = FALSE;
            STALL:;
                if ((ENABLE_DEBUG_MESSAGES) || (cpu->command == DISPLAY) || (cpu->command == SINGLE_STEP))
                {
                    print_stage_content("Decode/RF", &cpu->decode);
                }
            }
        }
    }
}

/*
 * Multiplier FU Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_multiplier_FU(APEX_CPU *cpu)
{
    if (cpu->multiplier.has_insn)
    {
        if ((cpu->multiplier.stall == 1) || (cpu->multiplier.stall == 2))
        { /* Incase three cycles are completed process the instruction*/
            if (cpu->multiplier.cycle == 2)
            {
                cpu->multiplier.result_buffer = cpu->multiplier.rs1_value * cpu->multiplier.rs2_value;
                /* Set the zero flag based on the result buffer */
                if (cpu->multiplier.result_buffer == 0)
                {
                    cpu->zero_flag = TRUE;
                }
                else
                {
                    cpu->zero_flag = FALSE;
                }
                cpu->multiplier.stall = 2;
                /* Copy data from execute latch to memory latch*/
                if (cpu->instruction_queue[cpu->Front] == cpu->multiplier.number)
                {
                    cpu->multiplier.stall = 0;
                    if (ENABLE_DEBUG_MESSAGES)
                    {
                        printf("MJXX: value to be deleted from multiplier:%d\n", cpu->multiplier.number);
                    }
                    cpu->writeback = cpu->multiplier;
                    cpu->multiplier.has_insn = FALSE;
                }
                if ((ENABLE_DEBUG_MESSAGES) || (cpu->command == DISPLAY) || (cpu->command == SINGLE_STEP))
                {
                    print_stage_content("Multiplier FU", &cpu->multiplier);
                }
            }
            else
            {
                cpu->multiplier.cycle++;
                if ((ENABLE_DEBUG_MESSAGES) || (cpu->command == DISPLAY) || (cpu->command == SINGLE_STEP))
                {
                    print_stage_content("Multiplier FU", &cpu->multiplier);
                }
            }
        }
        else
        {
            cpu->multiplier.stall = 1;
            cpu->multiplier.cycle = 1;

            /* Execute logic based on instruction type **/

            if ((ENABLE_DEBUG_MESSAGES) || (cpu->command == DISPLAY) || (cpu->command == SINGLE_STEP))
            {
                print_stage_content("Multiplier FU", &cpu->multiplier);
            }
        }
    }
}

/*
 * Multiplier FU Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_load_store_FU(APEX_CPU *cpu)
{
    if (cpu->load_store.has_insn)
    {
        if ((cpu->load_store.stall == 1) || (cpu->load_store.stall == 2))
        { /* Incase four cycles are completed process the instruction*/
            if (cpu->load_store.cycle == 3)
            {
                switch (cpu->load_store.opcode)
                {
                case OPCODE_LOAD:
                {
                    /* Read from data memory */
                    cpu->load_store.memory_address = cpu->load_store.rs2_value + cpu->load_store.imm;
                    cpu->load_store.result_buffer = cpu->data_memory[cpu->load_store.memory_address];
                    break;
                }
                case OPCODE_STORE:
                {
                    /* Read from data memory */
                    cpu->load_store.memory_address = cpu->load_store.rs2_value + cpu->load_store.imm;
                    cpu->data_memory[cpu->load_store.memory_address] = cpu->load_store.rs1_value;
                    break;
                }
                case OPCODE_LDR:
                {
                    /* Read from data memory */
                    cpu->load_store.memory_address = cpu->load_store.rs2_value + cpu->load_store.rs1_value;
                    cpu->load_store.result_buffer = cpu->data_memory[cpu->load_store.memory_address];
                    break;
                }
                case OPCODE_STR:
                {
                    /* Read from data memory */
                    cpu->load_store.memory_address = cpu->load_store.rs1_value + cpu->load_store.rs2_value;
                    cpu->data_memory[cpu->load_store.memory_address] = cpu->load_store.rs3_value;
                    break;
                }
                }
                cpu->load_store.stall = 2;
                if (cpu->instruction_queue[cpu->Front] == cpu->load_store.number)
                {
                    /* Copy data from execute latch to memory latch*/
                    cpu->load_store.stall = 0;
                    if (ENABLE_DEBUG_MESSAGES)
                    {
                        printf("MJXX: value to be deleted from LOAD/STORE:%d\n", cpu->load_store.number);
                    }
                    cpu->writeback = cpu->load_store;
                    cpu->load_store.has_insn = FALSE;
                    if ((ENABLE_DEBUG_MESSAGES) || (cpu->command == DISPLAY) || (cpu->command == SINGLE_STEP))
                    {
                        print_stage_content("Load/Store FU", &cpu->load_store);
                    }
                }
            }
            else
            {
                cpu->load_store.cycle++;
                if ((ENABLE_DEBUG_MESSAGES) || (cpu->command == DISPLAY) || (cpu->command == SINGLE_STEP))
                {
                    print_stage_content("Load/Store FU", &cpu->load_store);
                }
            }
        }
        else
        {
            cpu->load_store.stall = 1;
            cpu->load_store.cycle = 1;

            if ((ENABLE_DEBUG_MESSAGES) || (cpu->command == DISPLAY) || (cpu->command == SINGLE_STEP))
            {
                print_stage_content("Load/Store FU", &cpu->load_store);
            }
        }
    }
}

/*
 * Integer FU Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static void
APEX_integer_FU(APEX_CPU *cpu)
{
    if (cpu->integer.has_insn)
    {
        if ((cpu->integer.stall == 1) || (cpu->integer.stall == 2))
        { /* Proceed if instruction is at start of queue*/
            if (cpu->instruction_queue[cpu->Front] == cpu->integer.number)
            {
                goto ALLOW_INTEGER;
            }
        }
        else
        { /* Execute logic based on instruction type */
            cpu->integer.stall = 2;
            /* Copy data from execute latch to memory latch*/
            /* Proceed if instruction is at start of queue*/
            if (cpu->instruction_queue[cpu->Front] == cpu->integer.number)
            {
            ALLOW_INTEGER:;
                cpu->integer.stall = 0;
                switch (cpu->integer.opcode)
                {
                case OPCODE_ADD:
                {
                    cpu->integer.result_buffer = cpu->integer.rs1_value + cpu->integer.rs2_value;
                    /* Set the zero flag based on the result buffer */
                    if (cpu->integer.result_buffer == 0)
                    {
                        cpu->zero_flag = TRUE;
                    }
                    else
                    {
                        cpu->zero_flag = FALSE;
                    }
                    break;
                }
                case OPCODE_ADDL:
                {
                    cpu->integer.result_buffer = cpu->integer.rs1_value + cpu->integer.imm;
                    /* Set the zero flag based on the result buffer */
                    if (cpu->integer.result_buffer == 0)
                    {
                        cpu->zero_flag = TRUE;
                    }
                    else
                    {
                        cpu->zero_flag = FALSE;
                    }
                    break;
                }
                case OPCODE_SUB:
                {
                    cpu->integer.result_buffer = cpu->integer.rs1_value - cpu->integer.rs2_value;
                    /* Set the zero flag based on the result buffer */
                    if (cpu->integer.result_buffer == 0)
                    {
                        cpu->zero_flag = TRUE;
                    }
                    else
                    {
                        cpu->zero_flag = FALSE;
                    }
                    break;
                }
                case OPCODE_SUBL:
                {
                    cpu->integer.result_buffer = cpu->integer.rs1_value - cpu->integer.imm;
                    /* Set the zero flag based on the result buffer */
                    if (cpu->integer.result_buffer == 0)
                    {
                        cpu->zero_flag = TRUE;
                    }
                    else
                    {
                        cpu->zero_flag = FALSE;
                    }
                    break;
                }
                case OPCODE_DIV:
                {
                    cpu->integer.result_buffer = cpu->integer.rs1_value / cpu->integer.rs2_value;
                    /* Set the zero flag based on the result buffer */
                    if (cpu->integer.result_buffer == 0)
                    {
                        cpu->zero_flag = TRUE;
                    }
                    else
                    {
                        cpu->zero_flag = FALSE;
                    }
                    break;
                }
                case OPCODE_AND:
                {
                    cpu->integer.result_buffer = cpu->integer.rs1_value & cpu->integer.rs2_value;
                    /* Set the zero flag based on the result buffer */
                    if (cpu->integer.result_buffer == 0)
                    {
                        cpu->zero_flag = TRUE;
                    }
                    else
                    {
                        cpu->zero_flag = FALSE;
                    }
                    break;
                }
                case OPCODE_OR:
                {
                    cpu->integer.result_buffer = cpu->integer.rs1_value | cpu->integer.rs2_value;
                    /* Set the zero flag based on the result buffer */
                    if (cpu->integer.result_buffer == 0)
                    {
                        cpu->zero_flag = TRUE;
                    }
                    else
                    {
                        cpu->zero_flag = FALSE;
                    }
                    break;
                }
                case OPCODE_XOR:
                {
                    cpu->integer.result_buffer = cpu->integer.rs1_value ^ cpu->integer.rs2_value;
                    /* Set the zero flag based on the result buffer */
                    if (cpu->integer.result_buffer == 0)
                    {
                        cpu->zero_flag = TRUE;
                    }
                    else
                    {
                        cpu->zero_flag = FALSE;
                    }
                    break;
                }

                case OPCODE_CMP:
                {
                    /*   cpu->integer.result_buffer = cpu->integer.rs1_value - cpu->integer.rs2_value;*/
                    /* Set the zero flag based on the result buffer */
                    if ((cpu->integer.rs1_value - cpu->integer.rs2_value) == 0)
                    {
                        cpu->zero_flag = TRUE;
                    }
                    else
                    {
                        cpu->zero_flag = FALSE;
                    }
                    break;
                }

                case OPCODE_BZ:
                {
                    if (cpu->zero_flag == TRUE)
                    {
                        /* Calculate new PC, and send it to fetch unit */
                        cpu->pc = cpu->integer.pc + cpu->integer.imm;

                        /* Since we are using reverse callbacks for pipeline stages,
                         * this will prevent the new instruction from being fetched in the current cycle*/
                        cpu->fetch_from_next_cycle = TRUE;

                        /* Flush previous stages */
                        cpu->decode.stall = 0;
                        cpu->decode.has_insn = FALSE;

                        /* Make sure fetch stage is enabled to start fetching from new PC */
                        cpu->fetch.has_insn = TRUE;
                    }
                    break;
                }

                case OPCODE_BNZ:
                {
                    if (cpu->zero_flag == FALSE)
                    {
                        /* Calculate new PC, and send it to fetch unit */
                        cpu->pc = cpu->integer.pc + cpu->integer.imm;

                        /* Since we are using reverse callbacks for pipeline stages,
                         * this will prevent the new instruction from being fetched in the current cycle*/
                        cpu->fetch_from_next_cycle = TRUE;

                        /* Flush previous stages */
                        cpu->decode.stall = 0;
                        cpu->decode.has_insn = FALSE;

                        /* Make sure fetch stage is enabled to start fetching from new PC */
                        cpu->fetch.has_insn = TRUE;
                    }
                    break;
                }

                case OPCODE_MOVC:
                {
                    cpu->integer.result_buffer = cpu->integer.imm;

                    /* Set the zero flag based on the result buffer */
                    if (cpu->integer.result_buffer == 0)
                    {
                        cpu->zero_flag = TRUE;
                    }
                    else
                    {
                        cpu->zero_flag = FALSE;
                    }
                    break;
                }
                }
                if (ENABLE_DEBUG_MESSAGES)
                {
                    printf("MJXX: value to be deleted from integer:%d\n", cpu->integer.number);
                }
                cpu->writeback = cpu->integer;
                cpu->integer.has_insn = FALSE;
            }
            else
            { /* Stall if instruction is not at start of queue*/
                cpu->integer.stall = 1;
            }
        }

        if ((ENABLE_DEBUG_MESSAGES) || (cpu->command == DISPLAY) || (cpu->command == SINGLE_STEP))
        {
            print_stage_content("Integer FU", &cpu->integer);
        }
    }
}
/*
 * Writeback Stage of APEX Pipeline
 *
 * Note: You are free to edit this function according to your implementation
 */
static int
APEX_writeback(APEX_CPU *cpu)
{
    if (cpu->writeback.has_insn)
    {
        /* Write result to register file based on instruction type */
        switch (cpu->writeback.opcode)
        {
        case OPCODE_ADD:
        {
            cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
            break;
        }

        case OPCODE_ADDL:
        {
            cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
            break;
        }

        case OPCODE_SUB:
        {
            cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
            break;
        }

        case OPCODE_SUBL:
        {
            cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
            break;
        }
        case OPCODE_MUL:
        {
            cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
            break;
        }

        case OPCODE_DIV:
        {
            cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
            break;
        }

        case OPCODE_AND:
        {
            cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
            break;
        }

        case OPCODE_OR:
        {
            cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
            break;
        }
        case OPCODE_XOR:
        {
            cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
            break;
        }
        case OPCODE_LOAD:
        {
            cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
            break;
        }

        case OPCODE_STORE:
        {
            break;
        }

        case OPCODE_LDR:
        {
            cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
            break;
        }

        case OPCODE_STR:
        {
            break;
        }

        case OPCODE_MOVC:
        {
            cpu->regs[cpu->writeback.rd] = cpu->writeback.result_buffer;
            break;
        }
        case OPCODE_CMP:
        {
            break;
        }
        }
        dequeue(cpu);
        if (ENABLE_DEBUG_MESSAGES)
        {
            show(cpu);
        }

        cpu->insn_completed++;
        /* MJXX simple scoreboarding logic */
        /* Reset the destination state indicator once execution of instruction is completed */
        cpu->state_regs[cpu->writeback.rd] = 0;
        cpu->writeback.has_insn = FALSE;

        if ((ENABLE_DEBUG_MESSAGES) || (cpu->command == DISPLAY) || (cpu->command == SINGLE_STEP))
        {
            print_stage_content("Writeback", &cpu->writeback);
        }

        if (cpu->writeback.opcode == OPCODE_HALT)
        {
            /* Stop the APEX simulator */
            return TRUE;
        }
    }
    /* Default */
    return 0;
}
/*
 * This function creates and initializes APEX cpu.
 *
 * Note: You are free to edit this function according to your implementation
 */
APEX_CPU *
APEX_cpu_init(const char *filename)
{
    int i;
    APEX_CPU *cpu;

    if (!filename)
    {
        return NULL;
    }

    cpu = calloc(1, sizeof(APEX_CPU));

    if (!cpu)
    {
        return NULL;
    }

    /* Initialize PC, Registers and all pipeline stages */
    cpu->pc = 4000;
    memset(cpu->regs, 0, sizeof(int) * REG_FILE_SIZE);
    memset(cpu->state_regs, 0, sizeof(int) * REG_FILE_SIZE);
    memset(cpu->data_memory, 0, sizeof(int) * DATA_MEMORY_SIZE);
    memset(cpu->instruction_queue, 0, sizeof(int) * QUEUE_SIZE);
    cpu->Front = -1;
    cpu->Rear = -1;
    cpu->single_step = ENABLE_MULTIPLE_STEP;

    /* Parse input file and create code memory */
    cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);
    if (!cpu->code_memory)
    {
        free(cpu);
        return NULL;
    }

    if ((ENABLE_DEBUG_MESSAGES) || (cpu->command == INITIALIZE))
    {
        fprintf(stderr,
                "APEX_CPU: Initialized APEX CPU, loaded %d instructions\n",
                cpu->code_memory_size);
        fprintf(stderr, "APEX_CPU: PC initialized to %d\n", cpu->pc);
        fprintf(stderr, "APEX_CPU: Printing Code Memory\n");
        printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode_str", "rd", "rs1", "rs2",
               "imm");

        for (i = 0; i < cpu->code_memory_size; ++i)
        {
            printf("%-9s %-9d %-9d %-9d %-9d\n", cpu->code_memory[i].opcode_str,
                   cpu->code_memory[i].rd, cpu->code_memory[i].rs1,
                   cpu->code_memory[i].rs2, cpu->code_memory[i].imm);
        }
    }

    /* To start fetch stage */
    cpu->fetch.has_insn = TRUE;
    return cpu;
}

int APEX_cpu_simulator(const char *command)
{
    if (strcmp(command, "initialize") == 0)
    {
        return INITIALIZE;
    }
    else if (strcmp(command, "simulate") == 0)
    {

        return SIMULATE;
    }
    else if (strcmp(command, "single_step") == 0)
    {

        return SINGLE_STEP;
    }
    else if (strcmp(command, "display") == 0)
    {

        return DISPLAY;
    }
    else if (strcmp(command, "showmem") == 0)
    {

        return SHOWMEM;
    }
    else
    {
        return 0;
    }
}

/*
 * APEX CPU simulation loop
 *
 * Note: You are free to edit this function according to your implementation
 */
void APEX_cpu_run(APEX_CPU *cpu)
{
    char user_prompt_val;
    int user_prompt_cycle = 0;
    int temp_cycle = 0;

    while (TRUE)
    {
        if ((ENABLE_DEBUG_MESSAGES) || (cpu->command == DISPLAY) || (cpu->command == SINGLE_STEP))
        {
            printf("--------------------------------------------\n");
            printf("Clock Cycle #: %d\n", cpu->clock);
            printf("--------------------------------------------\n");
        }

        if (APEX_writeback(cpu))
        {
            /* Halt in writeback stage */
            if (cpu->command == SHOWMEM)
            {
                printf("MEM[%-2d]=%-2d ", cpu->command_2, cpu->data_memory[cpu->command_2]);
                printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
            }
            else
            {
                print_reg_file(cpu);
                print_memory_file(cpu);
                printf("APEX_CPU: Simulation Complete, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
            }
            break;
        }
        APEX_load_store_FU(cpu);
        APEX_multiplier_FU(cpu);
        APEX_integer_FU(cpu);
        APEX_decode(cpu);
        APEX_fetch(cpu);
        print_reg_file(cpu);
        print_memory_file(cpu);
        if (ENABLE_DEBUG_MESSAGES)
        {
            print_reg_file(cpu);
        }
        if ((cpu->command_2 != 0) && (cpu->command != SHOWMEM))
        {
            if (cpu->clock == cpu->command_2)
            {
                print_reg_file(cpu);
                print_memory_file(cpu);
                printf("Enter additional cycle number to run or 0 to quit:\n");
                scanf("%d", &user_prompt_cycle);

                if (user_prompt_cycle == 0)
                {
                    print_reg_file(cpu);
                    print_memory_file(cpu);
                    printf("APEX_CPU: Simulation Stopped, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
                    break;
                }
                temp_cycle = cpu->command_2;
                temp_cycle = temp_cycle + user_prompt_cycle;
                cpu->command_2 = temp_cycle;
            }
        }
        if (cpu->single_step)
        {
            printf("Press any key to advance CPU Clock or <q> to quit:\n");
            scanf("%c", &user_prompt_val);

            if ((user_prompt_val == 'Q') || (user_prompt_val == 'q'))
            {
                print_reg_file(cpu);
                print_memory_file(cpu);
                printf("APEX_CPU: Simulation Stopped, cycles = %d instructions = %d\n", cpu->clock, cpu->insn_completed);
                break;
            }
        }

        cpu->clock++;
    }
}

/*
 * This function deallocates APEX CPU.
 *
 * Note: You are free to edit this function according to your implementation
 */
void APEX_cpu_stop(APEX_CPU *cpu)
{
    free(cpu->code_memory);
    free(cpu);
}
