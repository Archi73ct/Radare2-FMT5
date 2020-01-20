typedef struct {
    ut8 opcode;
    ut8 dst_req;
    ut8 src1_req;
    ut8 src2_req;
    int8_t se;
    int8_t off;
} fmt5_instr_R_t;

typedef struct {
    ut8 opcode;
    ut8 dst_req;
    ut8 src1_req;
    ut8 src2_req;
    ut8 snm;
} fmt5_instr_C_t;

typedef struct {
    ut8 opcode;
    ut8 dst_req;
    ut8 src1_req;
    ut16 j_off;
    ut8 snm;
} fmt5_instr_J_t;

typedef struct {
    ut8 opcode;
    ut8 dst_req;
    ut16 bval;
    ut8 shv;
} fmt5_instr_I_t;

typedef struct {
    ut8 opcode;
    ut8 dst_req;
    ut8 src1_req;
    ut8 src2_req;
    ut8 blm;
    ut8 shm;
    ut8 shv;
} fmt5_instr_M_t;

typedef struct {
    char *name;
    char fmt;
} fmt5_op_t;

static const fmt5_op_t fmt5_opcodes[] = {
    // LOAD
    {"ldb",  'R'},
    {"ldh",  'R'},
    {"ldw",  'R'},
    {"lde",  'R'},
    // STORE
    {"stb",  'R'},
    {"sth",  'R'},
    {"stw",  'R'},
    {"ste",  'R'},
    // ALU 1
    {"add",  'R'},
    {"mul",  'R'},
    {"div",  'R'},
    {"nor",  'R'},
    // ALU 2
    {"mask", 'M'},
    {"alu5", '-'},
    {"alu6", '-'},
    {"alu7", '-'},
    // MOV 1
    {"movi", 'I'},
    {"addi", 'I'},
    {"cmov", 'C'},
    {"cmp",  'C'},
    // MOV 2
    {"mov4", '-'},
    {"mov5", '-'},
    {"mov6", '-'},
    {"cjmp", 'J'},
    // IO
    {"in",   'R'},
    {"out",  'R'},
    {"dskr", 'R'},
    {"dskw", 'R'},
    // SYS
    {"sys0", '-'},
    {"sys",  'R'},
    {"iret", 'R'},
    {"halt", 'R'}
};

typedef struct {
    RAsm* a;
    RAsmOp* rop;
    instr_t b;
    ut8 opc;
    fmt5_op_t op;
} fmt5_instr_t;

// Type for the analysis plugin
typedef struct {
    RAnal* a;
    RAnalOp* rop;
    instr_t b;
    ut8 opc;
    fmt5_op_t op;
    fmt5_instr_C_t instr_C;
    fmt5_instr_I_t instr_I;
    fmt5_instr_J_t instr_J;
    fmt5_instr_R_t instr_R;
    char esil[64]; // Hold current instr esil implementation
} fmt5_anal_instr_t;

const char* jmp_conds[] = {"NZ", "LE", "LT", "EQ", "AZ", "GT", "GE", "NE", "X3", "SLE","SLT", "X4", "X5", "SGT", "SGE", "X6"};