Femtium Instruction Reference
=============================

Instruction Format
------------------

Each Femtium instruction is encoded as a 32-bit *big endian* word, in
one of the following formats:

| form | canonical name | bit layout                         |
|------|----------------|------------------------------------|
| `R`  | Register       | `OOOOOrrrrrrxxxxxxyyyyyyEoooooooo` |
| `C`  | Conditional    | `OOOOOrrrrrrxxxxxxyyyyyy_____cccc` |
| `J`  | Jump           | `OOOOOrrrrrrxxxxxxjjjjjjjjjj_cccc` |
| `I`  | Immediate      | `OOOOOrrrrrriiiiiiiiiiiiiiiisssss` |
| `M`  | Mask           | `OOOOOrrrrrrxxxxxxyyyyyyBBSSsssss` |

A 32-bit instruction is subdivided into a number of possible `fields`,
as can be seen above (`O`, `r`, `x`, `y`, `E`, `o`, `c`, `j`, `i`,
`s`, `B`, `S`). Different instructions use different combinations of
these possible fields, but the *meaning* and *position* of each named
field never changes.

For example, most instructions use the `x` field, which denotes the
index of the first source register. As can be seen in the table above,
`x` is always encoded as bits `20-15`, and always refers to the src1
register index.

Field reference
---------------

| field | bits  | description                | bit layout                         |
|-------|-------|----------------------------|------------------------------------|
| `O`   | 31-27 | instruction opcode         | `OOOOO...........................` |
| `r`   | 26-21 | destination register index | `.....rrrrrr.....................` |
| `x`   | 20-15 | source 1 register index    | `...........xxxxxx...............` |
| `y`   | 14-9  | source 2 register index    | `.................yyyyyy.........` |
| `E`   | 8     | sign/zero-extension        | `.......................E........` |
| `o`   | 7-0   | offset                     | `........................oooooooo` |
| `c`   | 3-0   | compare mode selection     | `............................cccc` |
| `i`   | 21-5  | immediate value (unsigned) | `...........iiiiiiiiiiiiiiii.....` |
| `s`   | 4-0   | shift amount               | `...........................sssss` |

Format: R
---------

| form | bit 31-27 | bit 26-21 | bit 20-15 | bit 14-9 | bit 8          | bit 7-0     |
|------|-----------|-----------|-----------|----------|----------------|-------------|
| `R`  | `opcode`  | dst reg   | src1 reg  | src2 reg | sign extension | `i8` offset |

Format: C
---------

| form | bit 31-27 | bit 26-21 | bit 20-15 | bit 14-9 | bit 8-4     | bit 3       | bit 2      | bit 1-0  |
|------|-----------|-----------|-----------|----------|-------------|-------------|------------|----------|
| `C`  | `opcode`  | dst reg   | src1 reg  | src2 reg | reserved    | [Sign]      | [Negation] | [Mode]   |
|      |           |           |           |          | (must be 0) | 0: unsigned | 0: normal  | cmp code |
|      |           |           |           |          |             | 1: signed   | 1: negated |          |

Format: J
----------

| form | bit 31-27 | bit 26-21 | bit 20-15 | bit 14-5                                 | bit 4    | bit 3     | bit 2      | bit 1-0 |
|------|-----------|-----------|-----------|------------------------------------------|----------|-----------|------------|---------|
| `J`  | `opcode`  | dst reg   | src1 reg  | (signed) jump offset                     | reserved | [Sign]    | [Negation] | [Mode]  |
|      |           |           |           | offset represents number of 32-bit words |          | 0: normal | cmp code   |         |
|      |           |           |           | relative to instruction pointer          |          | 1: signed | 1: negated |         |

Format: I
---------

| form | bit 31-27 | bit 27-22 | bit 21-5         | bit 4-0     |
|------|-----------|-----------|------------------|-------------|
| `I`  | `opcode`  | dst reg   | `u16` base value | shift value |

Format: M
----------

| form | bit 31-27 | bit 26-21 | bit 20-15 | bit 14-9 | bit 8-7    | bit 6-5    | bit 4-0     |
|------|-----------|-----------|-----------|----------|------------|------------|-------------|
| `M`  | `opcode`  | dst reg   | src1 reg  | src2 reg | blend mode | shift mode | shift value |

The Sign-Negation-Mode (`c` field) encoding:
============================================

| Sign | Neg | Mode | Name | semantics      |
|------|-----|------|------|----------------|
| 0    | 0   | 00   | NZ   | `src2 != 0`    |
| 0    | 0   | 01   | LE   | `src1 <= src2` |
| 0    | 0   | 10   | LT   | `src1 <  src2` |
| 0    | 0   | 11   | EQ   | `src1 == src2` |
| 0    | 1   | 00   | AZ   | `src2 == 0`    |
| 0    | 1   | 01   | GT   | `src1 >  src2` |
| 0    | 1   | 10   | GE   | `src1 >= src2` |
| 0    | 1   | 11   | NE   | `src1 != src2` |
| 1    | 0   | 00   | X3   | `UNDEFINED3`   |
| 1    | 0   | 01   | SLE  | `src1 <= src2` |
| 1    | 0   | 10   | SLT  | `src1 <  src2` |
| 1    | 0   | 11   | X4   | `UNDEFINED4`   |
| 1    | 1   | 00   | X5   | `UNDEFINED5`   |
| 1    | 1   | 01   | SGT  | `src1 >  src2` |
| 1    | 1   | 10   | SGE  | `src1 >= src2` |
| 1    | 1   | 11   | X6   | `UNDEFINED6`   |

The M-format encoding:
======================

The M-format instruction (MASK) deal with masking operations. It takes
a source and destination register, and computes the following expression:


```
maskval = shift(regs[y], shiftmode, shiftval)
regs[r] = blend(regs[x], blendmode, maskval)
```

See the tables below for the behaviour of `blend()` and `shift()`.


| shiftmode | value | desc                                       |
|-----------|-------|--------------------------------------------|
| `SM_SHL`  | 0b00  | Shift Left                                 |
| `SM_SHR`  | 0b01  | Shift Right, (logical / zero extension)    |
| `SM_SAR`  | 0b10  | Shift Right, (arithmetic / sign extension) |
| `SM_X1`   | 0b11  | <reserved for future use>                  |


| blendmode | value | desc                            |
|-----------|-------|---------------------------------|
| `BL_MOV`  | 0b00  | `regs[r] = maskval`             |
| `BL_AND`  | 0b01  | `regs[r] = regs[r] AND maskval` |
| `BL_OR`   | 0b10  | `regs[r] = regs[r] OR  maskval` |
| `BL_XOR`  | 0b11  | `regs[r] = regs[r] XOR maskval` |

Instruction Reference
---------------------

Femtium instruction are divided into 8 groups of 4 instructions:

| Opcode | Group | Instr  | Fmt | Pseudo-code                                                                 | Note |
|--------|-------|--------|-----|-----------------------------------------------------------------------------|------|
| 0x00   | Load  | `LDB`  | R   | `regs[r] = extend(mem[regs[x] + regs[y] + o], E)`                           |      |
| 0x01   | -     | `LDH`  | R   | `regs[r] = extend(mem[regs[x] + regs[y] + o], E)`                           |      |
| 0x02   | -     | `LDW`  | R   | `regs[r] = mem[regs[x] + regs[y] + o]`                                      |      |
| 0x03   | -     | `LDE`  | R   | <reserved for future use>                                                   |      |
|        |       |        |     |                                                                             |      |
| 0x04   | Store | `STB`  | R   | Write `regs[r]` to `[b] + [c] + o` ( i8 or u8)                              |      |
| 0x05   | -     | `STH`  | R   | Write `regs[r]` to `[b] + [c] + o` (i16 or u16)                             |      |
| 0x06   | -     | `STW`  | R   | Write `regs[r]` to `[b] + [c] + o` (u32)                                    |      |
| 0x07   | -     | `STE`  | R   | <reserved for future use>                                                   |      |
|        |       |        |     |                                                                             |      |
| 0x08   | ALU1  | `ADD`  | R   | `regs[r] = regs[x] + (regs[y] + o)`                                         |      |
| 0x09   | -     | `MUL`  | R   | `regs[r] = regs[x] * (regs[y] + o)`                                         |      |
| 0x0a   | -     | `DIV`  | R   | `regs[r] = regs[x] / (regs[y] + o)`                                         |      |
| 0x0b   | -     | `NOR`  | R   | `regs[r] = ~(regs[x] OR regs[y] OR o)`                                      |      |
|        |       |        |     |                                                                             |      |
| 0x0c   | ALU2  | `MASK` | M   | `regs[r] = blend(regs[x], blendmode, shift(regs[y], shiftmode, shiftval))`  |      |
| 0x0d   | -     | `ALU5` | -   | <reserved for future use>                                                   |      |
| 0x0e   | -     | `ALU6` | -   | <reserved for future use>                                                   |      |
| 0x0f   | -     | `ALU7` | -   | <reserved for future use>                                                   |      |
|        |       |        |     |                                                                             |      |
| 0x10   | MOV1  | `MOVI` | I   | `regs[r] = (i SHR s)`                                                       |      |
| 0x11   | -     | `ADDI` | I   | `regs[r] += (i SHR s)`                                                      |      |
| 0x12   | -     | `CMOV` | C   | `if (cond) { regs[r] = regs[x] }`                                           |      |
| 0x13   | -     | `CMP`  | C   | `if (cond) { regs[r] = 1 } else { regs[r] = 0 }`                            |      |
|        |       |        |     |                                                                             |      |
| 0x14   | MOV2  | `MOV4` | -   | <reserved for future use>                                                   |      |
| 0x15   | -     | `MOV5` | -   | <reserved for future use>                                                   |      |
| 0x16   | -     | `MOV6` | -   | <reserved for future use>                                                   |      |
| 0x17   | -     | `CJMP` | J   | `if (cond) { regs[IP] += (j * 4) }`                                         |      |
|        |       |        |     |                                                                             |      |
| 0x18   | IO    | `IN`   | R   | Read `regs[r]` from device `regs[x]` on port `regs[y] + o`                  |      |
| 0x19   | -     | `OUT`  | R   | Write `regs[r]` to device `regs[x]` on port `regs[y] + o`                   |      |
| 0x1a   | -     | `DSKR` | R   | Read block to memory at `regs[r]` from block `regs[x]` on device `regs[y]`  |      |
| 0x1b   | -     | `DSKW` | R   | Write block from memory at `regs[r]` to block `regs[x]` on device `regs[y]` |      |
|        |       |        |     |                                                                             |      |
| 0x1c   | SYS   | `SYS0` | -   | <reserved for future use>                                                   |      |
| 0x1d   | -     | `SYS`  | R   | Perform system call                                                         |      |
| 0x1e   | -     | `IRET` | R   | Return from interrupt. Restore register 62+63 and enable interrupts.        |      |
| 0x1f   | -     | `HALT` | R   | Halt machine.                                                               |      |
