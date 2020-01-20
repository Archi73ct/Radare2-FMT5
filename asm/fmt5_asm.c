#include <stdio.h>
#include <string.h>
#include <r_types.h>
#include <r_lib.h>
#include <r_asm.h>
#include <string.h>
#include <byteswap.h>

#include "../fmt5.h"
#include "../fmt5_ops.h"


static void fmt5_parse_R(fmt5_instr_t* fmt5_instr) {
    fmt5_instr_R_t op;
    instr_t b = fmt5_instr->b;
    char msg[0x7f] = {0};

    op.opcode   =  b >> 27;
    op.dst_req  = (b >> 21)   & 0b0111111;
    op.src1_req = (b >> 15)   & 0b0111111;
    op.src2_req = (b >> 9)    & 0b0111111;
    op.se       = (b >> 8)    & 0b0000001;
    op.off      =  b          & 0b1111111;

    switch(op.opcode) {
        case 0x0: {
            sprintf(msg, "%s %s = @%s + %s + %u], %u", fmt5_instr->op.name, regs[op.dst_req].name, regs[op.src1_req].name, regs[op.src2_req].name, op.off, op.se);
            break;
        }
        case 0x01: {
            sprintf(msg, "%s %s = @%s + %s + %u], %u", fmt5_instr->op.name, regs[op.dst_req].name, regs[op.src1_req].name, regs[op.src2_req].name, op.off, op.se);
            break;
        }
        case 0x02: {
            sprintf(msg, "%s %s = @%s + %s + %u]", fmt5_instr->op.name, regs[op.dst_req].name, regs[op.src1_req].name, regs[op.src2_req].name, op.off);
            break;
        }
        case 0x04: {
            sprintf(msg, "%s @%s + %s + %u = %s", fmt5_instr->op.name, regs[op.src1_req].name,regs[op.src2_req].name, op.off, regs[op.dst_req].name);
            break;
        }
        case 0x05: {
            sprintf(msg, "%s @%s + %s + %u = %s", fmt5_instr->op.name, regs[op.src1_req].name,regs[op.src2_req].name, op.off, regs[op.dst_req].name);
            break;
        }
        case 0x06: {
            sprintf(msg, "%s @%s + %s + %u = %s", fmt5_instr->op.name, regs[op.src1_req].name, regs[op.src2_req].name, op.off, regs[op.dst_req].name);
            break;
        }
        case 0x08: {
            sprintf(msg, "%s %s = %s + (%s + %u)", fmt5_instr->op.name, regs[op.dst_req].name, regs[op.src1_req].name, regs[op.src2_req].name, op.off);
            break;
        }
        case 0x09: {
            sprintf(msg, "%s %s = %s * (%s + %u)", fmt5_instr->op.name, regs[op.dst_req].name, regs[op.src1_req].name, regs[op.src2_req].name, op.off);
            break;
        }
        case 0x0a: {
            sprintf(msg, "%s %s = %s / (%s + %u)", fmt5_instr->op.name, regs[op.dst_req].name, regs[op.src1_req].name, regs[op.src2_req].name, op.off);
            break;
        }
        case 0x0b: {
            sprintf(msg, "%s %s = ~(%s OR %s OR %i)", fmt5_instr->op.name, regs[op.dst_req].name, regs[op.src1_req].name, regs[op.src2_req].name, op.off);
            break;
        }
        case 0x1d: {
            sprintf(msg, "SYSCALL");
            break;
        }
        default: {
            sprintf(msg, "UNK: %s", fmt5_instr->op.name);
        }
    }
    
    r_asm_op_set_asm(fmt5_instr->rop, msg);
}

static void fmt5_parse_I(fmt5_instr_t* fmt5_instr) {
    fmt5_instr_I_t op;
    instr_t b = fmt5_instr->b;
    char msg[0x7f] = {0};

    op.opcode   =  b >> 27;
    op.dst_req  = (b >> 21)   & 0b0111111;
    op.bval     = (b >> 5)    & 0b1111111111111111;
    op.shv      =  b          & 0b11111;

    switch(op.opcode) {
        case 0x10: {
            if (op.bval<<op.shv < 0x7f && op.bval<<op.shv >= 0x20) {
                sprintf(msg, "%s %s = %u << %u\t(%u: '%c')", fmt5_instr->op.name, regs[op.dst_req].name, op.bval, op.shv, op.bval<<op.shv, op.bval<<op.shv);
            } else {
                sprintf(msg, "%s %s = %u << %u\t(%u)", fmt5_instr->op.name, regs[op.dst_req].name, op.bval, op.shv, op.bval<<op.shv);
            }
            break;
        }
        case 0x11: {
            sprintf(msg, "%s %s += %u << %u\t(%u)", fmt5_instr->op.name, regs[op.dst_req].name, op.bval, op.shv, op.bval<<op.shv);
            break;
        }
        default: {
            sprintf(msg, "UNKi: %s, %u", fmt5_instr->op.name, (b>>27));
        }
    }
    
    r_asm_op_set_asm(fmt5_instr->rop, msg);
}

static void fmt5_parse_J(fmt5_instr_t* fmt5_instr) {
    fmt5_instr_J_t op;
    instr_t b = fmt5_instr->b;
    char msg[0x7f] = {0};

    op.opcode   =  b >> 27;
    op.dst_req  = (b >> 21)   & 0b0111111;
    op.src1_req = (b >> 15)   & 0b0111111;
    op.j_off    = (b >> 5)    & 0b111111111;
    op.snm      =  b          & 0b1111;

    switch(op.opcode) {
        case 0x17: {
            sprintf(msg, "%s\tif (%s %s %s) goto; %p", fmt5_instr->op.name, regs[op.dst_req].name, jmp_conds[op.snm], regs[op.src1_req].name, fmt5_instr->a->pc+(op.j_off*4));
            break;
        }
        default: {
            sprintf(msg, "UNKi: %s, %u", fmt5_instr->op.name, (b>>27));
        }
    }
    
    r_asm_op_set_asm(fmt5_instr->rop, msg);
}

static int disassemble(RAsm *a, RAsmOp *rop, const ut8* buf, int len) {
    char msg[10] = {0};
    fmt5_instr_t fmt5_instr;

    fmt5_instr.a = a;
    fmt5_instr.rop = rop;
    fmt5_instr.b = bswap_32(*(instr_t *)buf);
    fmt5_instr.opc = fmt5_instr.b >> 27;
    fmt5_instr.op = fmt5_opcodes[fmt5_instr.opc];

    switch(fmt5_instr.op.fmt) {
        case 'R': {
            fmt5_parse_R(&fmt5_instr);
            break;
        }
        case 'I': {
            fmt5_parse_I(&fmt5_instr);
            break;
        }
        case 'J': {
            fmt5_parse_J(&fmt5_instr);
            break;
        }
        default: {
            sprintf(msg, "UNK: %s", fmt5_instr.op.name);
            r_asm_op_set_asm(rop, msg);
        }
    }
    
    return rop->size;
}

RAsmPlugin r_asm_plugin_fmt5 = {
    .name = "fmt5",
    .desc = "FMT5 disassembly plugin",
    .arch = "fmt5",
    .bits = 32,
    .endian = R_SYS_ENDIAN_BIG,
    .disassemble = &disassemble
};

#ifndef R2_PLUGIN_INCORE
R_API RLibStruct radare_plugin = {
    .type = R_LIB_TYPE_ASM,
    .data = &r_asm_plugin_fmt5,
    .version = R2_VERSION
};
#endif
