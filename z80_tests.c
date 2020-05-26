#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "z80.h"

// MARK: simple memory interface
#define MEMORY_SIZE 0x10000
static uint8_t* memory = NULL;
static bool test_finished = 0;

static uint8_t rb(void* userdata, uint16_t addr) {
  return memory[addr];
}

static void wb(void* userdata, uint16_t addr, uint8_t val) {
  memory[addr] = val;
}

static int load_file(const char* filename, uint16_t addr) {
  FILE* f = fopen(filename, "rb");
  if (f == NULL) {
    fprintf(stderr, "error: can't open file '%s'.\n", filename);
    return 1;
  }

  // file size check:
  fseek(f, 0, SEEK_END);
  size_t file_size = ftell(f);
  rewind(f);

  if (file_size + addr >= MEMORY_SIZE) {
    fprintf(stderr, "error: file %s can't fit in memory.\n", filename);
    return 1;
  }

  // copying the bytes in memory:
  size_t result = fread(&memory[addr], sizeof(uint8_t), file_size, f);
  if (result != file_size) {
    fprintf(stderr, "error: while reading file '%s'\n", filename);
    return 1;
  }

  fclose(f);
  return 0;
}

static uint8_t in(z80* const z, uint8_t port) {
  uint8_t operation = z->c;

  // print a character stored in E
  if (operation == 2) {
    printf("%c", z->e);
  }
  // print from memory at (DE) until '$' char
  else if (operation == 9) {
    uint16_t addr = (z->d << 8) | z->e;
    do {
      printf("%c", rb(z, addr++));
    } while (rb(z, addr) != '$');
  }

  return 0xFF;
}

static void out(z80* const z, uint8_t port, uint8_t val) {
  test_finished = 1;
}

// MARK: test runner
static int run_test(
    z80* const z, const char* filename, unsigned long cyc_expected) {
  z80_init(z);
  z->read_byte = rb;
  z->write_byte = wb;
  z->port_in = in;
  z->port_out = out;
  memset(memory, 0, MEMORY_SIZE);

  if (load_file(filename, 0x100) != 0) {
    return 1;
  }
  printf("*** TEST: %s\n", filename);

  z->pc = 0x100;

  // inject "out 1,a" at 0x0000 (signal to stop the test)
  memory[0x0000] = 0xD3;
  memory[0x0001] = 0x00;

  // inject "in a,0" at 0x0005 (signal to output some characters)
  memory[0x0005] = 0xDB;
  memory[0x0006] = 0x00;
  memory[0x0007] = 0xC9;

  long nb_instructions = 0;

  test_finished = 0;
  while (!test_finished) {
    nb_instructions += 1;

    // warning: the following line will output dozens of GB of data.
    // z80_debug_output(z);

    z80_step(z);
  }

  long long diff = cyc_expected - z->cyc;
  printf("\n*** %lu instructions executed on %lu cycles"
         " (expected=%lu, diff=%lld)\n\n",
      nb_instructions, z->cyc, cyc_expected, diff);

  return cyc_expected != z->cyc;
}

int main(void) {
  memory = malloc(MEMORY_SIZE);
  if (memory == NULL) {
    return 1;
  }

  z80 cpu;

  // the following cycle counts have been retrieved from z80emu
  // (https://github.com/anotherlin/z80emu) for those exact roms
  int r = 0;
  r += run_test(&cpu, "roms/prelim.com", 8721LU);
  r += run_test(&cpu, "roms/zexdoc.cim", 46734978649LU);
  r += run_test(&cpu, "roms/zexall.cim", 46734978649LU);

  free(memory);
  return r != 0;
}
