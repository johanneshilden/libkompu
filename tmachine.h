#ifndef TMACHINE_H
#define TMACHINE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <inttypes.h>
#include <stdlib.h>

#define tm_int uint8_t
#define TAPE_BUFFER_MAX_MEM_SIZE (32 * 1024 * 1024)

typedef enum {
    TM_LEFT = 0,
    TM_RIGHT
} tm_machine_direction_t;

typedef enum {
    TM_TAPE_OK = 0,
    TM_TAPE_ENOMEM = -1
} tm_tape_buferror_t;

struct tm_machine_state
{
    struct tm_instruction *instrs;
    struct tm_machine_state *next;
};

struct tm_machine
{
    tm_int curr_state;
    struct tm_machine_state *states;
    tm_int state_count;
};

struct tm_instruction
{
    struct tm_instruction *next;
    tm_int symbol_in;
    tm_int symbol_out;
    tm_int state_out;
    tm_machine_direction_t direction;
};

struct tm_tape
{
    tm_int *data;
    size_t size;
    size_t asize;
    size_t unit;
    int p;
};

struct tm_machine *t_machine_new();
void t_machine_destroy(struct tm_machine *machine);

struct tm_machine_state *t_machine_add_state(struct tm_machine *machine);
void t_machine_insert_states(struct tm_machine *machine, size_t n);
struct tm_machine_state *t_machine_state_get(struct tm_machine *machine, tm_int at);

void t_machine_add_instruction(struct tm_machine *machine, tm_int state_in, tm_int state_out, tm_int symbol_in, tm_int symbol_out, uint8_t dir);

struct tm_tape *t_machine_tape_new();
void t_machine_tape_destroy(struct tm_tape *tape);

void t_machine_tape_append_symbol(struct tm_tape *tape, tm_int symbol);
void t_machine_tape_prepend_symbol(struct tm_tape *tape, tm_int symbol);

int8_t t_machine_run(struct tm_machine *machine, struct tm_tape *tape);

void t_machine_dump_tape(struct tm_tape *tape);

#endif // TMACHINE_H
