#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define MEM_SIZE 256
#define REG_COUNT 4
#define PROG_SIZE 256

typedef enum {
    NOP, MOV, ADD, SUB, STORE, HLT
} Opcode;

typedef struct {
    Opcode op;
    uint8_t dst;
    uint8_t src_or_imm;
} Instruction;

typedef struct {
    uint8_t R[REG_COUNT];
    uint16_t PC;
    uint8_t Z;
    uint8_t memory[MEM_SIZE];
} CPU;

// 執行 CPU
void execute(CPU *cpu, Instruction *program, int program_len) {
    cpu->PC = 0;
    while (cpu->PC < program_len) {
        Instruction *inst = &program[cpu->PC];
        switch(inst->op) {
            case MOV:
                cpu->R[inst->dst] = inst->src_or_imm;
                break;
            case ADD:
                cpu->R[inst->dst] += cpu->R[inst->src_or_imm];
                cpu->Z = (cpu->R[inst->dst] == 0);
                break;
            case SUB:
                cpu->R[inst->dst] -= cpu->R[inst->src_or_imm];
                cpu->Z = (cpu->R[inst->dst] == 0);
                break;
            case STORE:
                cpu->memory[inst->src_or_imm] = cpu->R[inst->dst];
                break;
            case HLT:
                return;
            case NOP:
            default:
                break;
        }
        cpu->PC++;
    }
}

// Helper: 將 assembly 文字轉 Instruction
int assemble(Instruction *program) {
    // 這裡寫死一個小程式
    // R0 = 10, R1 = 20, R0 = R0 + R1, 存入 memory[100], 停止
    int pc = 0;
    program[pc++] = (Instruction){MOV, 0, 10}; // MOV R0, 10
    program[pc++] = (Instruction){MOV, 1, 20}; // MOV R1, 20
    program[pc++] = (Instruction){ADD, 0, 1};  // ADD R0, R1
    program[pc++] = (Instruction){STORE, 0, 100}; // STORE R0 -> memory[100]
    program[pc++] = (Instruction){HLT, 0, 0}; // HLT
    return pc;
}

int main() {
    CPU cpu;
    memset(&cpu, 0, sizeof(CPU));

    Instruction program[PROG_SIZE];
    int program_len = assemble(program);

    execute(&cpu, program, program_len);

    printf("Registers:\n");
    for(int i=0; i<REG_COUNT; i++)
        printf("R%d = %d\n", i, cpu.R[i]);

    printf("Memory[100] = %d\n", cpu.memory[100]);

    return 0;
}
