#include <string.h>
#include <r_types.h>
#include <r_lib.h>
#include <r_asm.h>
#include <r_anal.h>
#include <byteswap.h>
#include "../fmt5.h"
#include "../fmt5_ops.h"

/* COMMENTS
 ESIL is ready to be implemented, a bit of work... :/
 REF https://radare.gitbooks.io/radare2book/content/disassembling/esil.html
 An ESIL string should be written for every instruction, check your results with
 > e asm.arch=fmt5
 > e asm.esil=true
 > pd
*/


static void fmt5_parse_R(fmt5_anal_instr_t* fmt5_instr) {
    fmt5_instr_R_t op;
    instr_t b = fmt5_instr->b;
    char msg[0x7f] = {0};
    op.opcode   =  b >> 27;
    op.dst_req  = (b >> 21)   & 0b0111111;
    op.src1_req = (b >> 15)   & 0b0111111;
    op.src2_req = (b >> 9)    & 0b0111111;
    op.se       = (b >> 8)    & 0b0000001;
    op.off      =  b          & 0b1111111;

    switch (op.opcode)
    {
    case 0x00:
        // LDB 
        esilprintf(fmt5_instr->rop, "%s,%s,+,%d,+,[],%s,=", regs[op.src1_req].name, regs[op.src2_req].name, op.off, regs[op.dst_req].name);
        break;
    case 0x01:
        // LDH 
        esilprintf(fmt5_instr->rop, "%s,%s,+,%d,+,[],%s,=", regs[op.src1_req].name, regs[op.src2_req].name, op.off, regs[op.dst_req].name);
        break;
    case 0x02:
        // LDW 
        esilprintf(fmt5_instr->rop, "%s,%s,+,%d,+,[],%s,=", regs[op.src1_req].name, regs[op.src2_req].name, op.off, regs[op.dst_req].name);
        break;
    case 0x04:
        //STB
        esilprintf(fmt5_instr->rop, "%s,0x%x,0x%x,+,%d,+,=[1]", regs[op.dst_req].name, op.src1_req, op.src2_req, op.off);
        break;
    case 0x05:
        //STH
        esilprintf(fmt5_instr->rop, "%s,0x%x,0x%x,+,%d,+,=[2]", regs[op.dst_req].name, op.src1_req, op.src2_req, op.off);
        break;
    case 0x06:
        //STW
        esilprintf(fmt5_instr->rop, "%s,0x%x,0x%x,+,%d,+,=[4]", regs[op.dst_req].name, op.src1_req, op.src2_req, op.off);
        break;
    case 0x08:
        //ADD
        // Horrible ESIL...
        // Special case to check if a number is negative or positive since esil can't handle signed stuff that well
        esilprintf(fmt5_instr->rop, "0,%s,%d,+,%s,+,<,?{,%s,%d,+,%s,+,%s,-,%s,=,BREAK,},%s,%d,+,%s,+,%s,=",regs[op.src2_req].name, op.off, regs[op.src1_req].name,regs[op.src2_req].name, op.off, regs[op.src1_req].name,regs[op.dst_req].name,regs[op.dst_req].name,regs[op.src2_req].name, op.off, regs[op.src1_req].name,regs[op.dst_req].name);
        
        break;
    case 0x09:
        //MUL
        esilprintf(fmt5_instr->rop, "0x%x,%s,+,%s,*,%s,=", op.off, regs[op.src2_req].name, regs[op.src1_req].name, regs[op.dst_req].name);
        break;
    case 0x0a:
        //DIV
        esilprintf(fmt5_instr->rop, "0x%x,%s,+,%s,/,%s,=", op.off, regs[op.src2_req].name, regs[op.src1_req].name, regs[op.dst_req].name);
        break;
    case 0x0b:
        //NOR
        // Not sure about this yet regs[r] = ~(regs[x] OR regs[y] OR o)
        esilprintf(fmt5_instr->rop, "%d,%s,|,%s,|,!,%s,=", op.off, regs[op.src2_req].name, regs[op.src1_req].name, regs[op.dst_req].name);
        break;
    default:
        esilprintf(fmt5_instr->rop, "");
        break;
    }
    fmt5_instr->instr_R = op;
    
}

static void fmt5_parse_I(fmt5_anal_instr_t* fmt5_instr) {
    fmt5_instr_I_t op;
    instr_t b = fmt5_instr->b;
    char msg[0x7f] = {0};

    op.opcode   =  b >> 27;
    op.dst_req  = (b >> 21)   & 0b0111111;
    op.bval     = (b >> 5)    & 0b111111111111111111;
    op.shv      =  b          & 0b11111;

    switch (op.opcode)
    {
    case 0x10:
        esilprintf(fmt5_instr->rop, "0x%x,0x%x,<<,%s,=", op.shv, op.bval, regs[op.dst_req].name);
        break;
    case 0x11:
        esilprintf(fmt5_instr->rop, "0x%x,0x%x,<<,%s,+=", op.shv, op.bval, regs[op.dst_req].name);
        break;

    default:
        esilprintf(fmt5_instr->rop, "");
        break;
    }

    fmt5_instr->instr_I = op;
    
}

static void fmt5_parse_J(fmt5_anal_instr_t* fmt5_instr) {
    fmt5_instr_J_t op;
    instr_t b = fmt5_instr->b;
    char msg[0x7f] = {0};

    op.opcode   =  b >> 27;
    op.dst_req  = (b >> 21)   & 0b0111111;
    op.src1_req = (b >> 15)   & 0b0111111;
    op.j_off    = (b >> 5)    & 0b111111111;
    op.snm      =  b          & 0b1111;

    fmt5_instr->instr_J = op;
}



// actual analysis part, other stuff is copy pasted
static int analyse(RAnal *a, RAnalOp *rop, ut64 addr, const ut8 *b, int len) {
    fmt5_anal_instr_t fmt5_instr;

    fmt5_instr.a = a;
    fmt5_instr.rop = rop;
    fmt5_instr.b = bswap_32(*(instr_t *)b);
    fmt5_instr.opc = fmt5_instr.b >> 27;
    fmt5_instr.op = fmt5_opcodes[fmt5_instr.opc];
    rop->size = 4;
    rop->type = R_ANAL_OP_TYPE_UNK;
    rop->nopcode=1;
    rop->addr = addr;
    // No ESIL before we understand the special purpose regs.
    // Types are for color coding, and arrows in case of jumps
    switch(fmt5_instr.op.fmt) {
        case 'R': {
            fmt5_parse_R(&fmt5_instr);
            switch (fmt5_instr.opc)
            {
                case 0x00: case 0x01: case 0x02: case 0x03:
                    rop->type = R_ANAL_OP_TYPE_LOAD;
                    break;
                case 0x04: case 0x05: case 0x06: case 0x07:
                    rop->type = R_ANAL_OP_TYPE_STORE;
                    break;
                case 0x08:
                    if(fmt5_instr.instr_R.dst_req == 63){
                        rop->type = R_ANAL_OP_TYPE_CALL;
                    }else
                    {
                        rop->type = R_ANAL_OP_TYPE_ADD;
                    }
                    break;
                case 0x09:
                    rop->type = R_ANAL_OP_TYPE_MUL;
                    break;
                case 0x0a:
                    rop->type = R_ANAL_OP_TYPE_DIV;
                    break;
                case 0x0b:
                    rop->type = R_ANAL_OP_TYPE_NOR;
                    break;
            }
            
            break;
        }
        case 'I': {
            fmt5_parse_I(&fmt5_instr);
            switch(fmt5_instr.opc){
                case 0x10:
                    rop->type = R_ANAL_OP_TYPE_MOV;
                    break;
                case 0x11:
                    rop->type = R_ANAL_OP_TYPE_ADD;
                    break;
            }
            break;
        }
        case 'J': {
            fmt5_parse_J(&fmt5_instr);
            rop->type = R_ANAL_OP_TYPE_CJMP;
            rop->jump = addr+(fmt5_instr.instr_J.j_off*4);
            rop->fail = addr+1*4;
            break;
        }
    }
    return rop->size;

}

static int set_reg_profile(RAnal *anal){
    const char *p = 
    "=A0    r03\n"
    "=A1    r04\n"
    "=A2    r05\n"
    "=LR    r59\n"
    "=PC    eip\n"
    "=SP    esp\n"
    "=SN    r06\n"
    "gpr r00 .32     0 0\n gpr r01 .32     4 0\n gpr r02 .32    8 0\n gpr r03 .32    12 0\n gpr r04 .32    16 0\n gpr r05 .32    20 0\n gpr r06 .32    24 0\n gpr r07 .32    28 0\n"
    "gpr r08 .32    32 0\n gpr r09 .32    36 0\n gpr r10 .32    40 0\n gpr r11 .32    44 0\n gpr r12 .32    48 0\n gpr r13 .32    52 0\n gpr r14 .32    56 0\n gpr r15 .32    60 0\n"
    "gpr r16 .32    64 0\n gpr r17 .32    68 0\n gpr r18 .32    72 0\n gpr r19 .32    76 0\n gpr r20 .32    80 0\n gpr r21 .32    84 0\n gpr r22 .32    88 0\n gpr r23 .32    92 0\n"
    "gpr r24 .32    96 0\n gpr r25 .32    100 0\n gpr r26 .32    104 0\n gpr r27 .32    108 0\n gpr r28 .32    112 0\n gpr r29 .32    116 0\n gpr r30 .32    120 0\n gpr r31 .32    124 0\n"
    "gpr r32 .32   128 0\n gpr r33 .32     132 0\n gpr r34 .32    136 0\n gpr r35 .32    140 0\n gpr r36 .32    144 0\n gpr r37 .32    148 0\n gpr r38 .32    152 0\n gpr r39 .32    156 0\n"
    "gpr r40 .32   160 0\n gpr r41 .32    164 0\n gpr r42 .32    168 0\n gpr r43 .32    172 0\n gpr r44 .32    176 0\n gpr r45 .32    180 0\n gpr r46 .32    184 0\n gpr r47 .32    188 0\n"
    "gpr r48 .32   192 0\n gpr r49 .32    196 0\n gpr r50 .32    200 0\n gpr r51 .32    204 0\n gpr r52 .32    208 0\n gpr r53 .32    212 0\n gpr r54 .32    216 0\n gpr r55 .32    220 0\n"
    "gpr r56 .32   224 0\n gpr r57 .32    228 0\n gpr r58 .32    232 0\n gpr r59 .32    236 0\n gpr r60 .32    240 0\n gpr r61 .32    244 0\n gpr esp .32    248 0\n gpr eip .32    252 0\n";
    // osv
    return r_reg_set_profile_string(anal->reg, p);
}

struct r_anal_plugin_t r_anal_plugin_fmt5 = {
    .name = "fmt5",
    .desc = "FMT5 analysis plugin",
    .arch = "fmt5",
    .bits = 32,
    .init = NULL,
    .fini = NULL,
    .op = &analyse,
    .set_reg_profile = &set_reg_profile,
    .fingerprint_bb = NULL,
    .fingerprint_fcn = NULL,
    .diff_bb = NULL,
    .diff_fcn = NULL,
    .diff_eval = NULL,
    .esil = true
};

#ifndef R2_PLUGIN_INCORE
R_API RLibStruct radare_plugin = {
    .type = R_LIB_TYPE_ANAL,
    .data = &r_anal_plugin_fmt5,
    .version = R2_VERSION
};
#endif