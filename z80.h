#ifndef Z80_Z80_H
#define Z80_Z80_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint8_t u8;
typedef int8_t s8;
typedef uint16_t u16;
typedef int32_t s32;

typedef struct z80 z80;
struct z80 {
    u8 a, b, c, d, e, h, l; // main registers
    u8 a_, b_, c_, d_, e_, h_, l_, f_; // alternate registers
    u8 i, r; // interrupt vector, memory refresh
    u16 pc, sp, ix, iy; // special purpose registers

    // flags: sign, zero, yf, half-carry, xf, parity/overflow, negative, carry
    bool sf, zf, yf, hf, xf, pf, nf, cf;

    u8 iff_delay;
    u8 interrupt_mode;
    bool iff1, iff2;
    bool halted;
    bool int_pending, nmi_pending;
    u8 int_data;

    u16 mem_ptr; // "wz" register

    unsigned long cyc; // cycle count (t-states)

    void* userdata;
    u8 (*read_byte)(void*, u16);
    void (*write_byte)(void*, u16, u8);
    u8 (*port_in)(z80*, u8);
    void (*port_out)(z80*, u8, u8);
};

void z80_init(z80* const z);
void z80_step(z80* const z);
void z80_debug_output(z80* const z);
void z80_gen_nmi(z80* const z);
void z80_gen_int(z80* const z, u8 data);

#endif // Z80_Z80_H
