#include <malloc.h>
#include <assert.h>
#include <string.h>
#include "tmachine.h"

/*!
 *  \struct tm_machine_state
 *
 *  \brief A Turing machine state.
 */

/*!
 *  \struct tm_machine
 *
 *  \brief The base struct representing a Turing machine.
 */

/*!
 *  \struct tm_instruction
 *
 *  \brief A Turing machine instruction.
 */

/*!
 *  \struct tm_tape
 *
 *  \brief A Turing machine tape.
 */

static void
t_machine_destroy_state(struct tm_machine_state *state)
{
    struct tm_instruction *instr, *next;
    instr = state->instrs;
    while (instr) {
        next = instr->next;
        free(instr);
        instr = next;
    }
    free(state);
}

static tm_tape_buferror_t
t_machine_tape_buffer_grow(struct tm_tape *tape, size_t n)
{
    size_t a;
    void *data;

    assert(tape && tape->unit);

    if (n > TAPE_BUFFER_MAX_MEM_SIZE)
        return TM_TAPE_ENOMEM;
    if (tape->asize >= n)
        return TM_TAPE_OK;
    a = tape->asize + tape->unit;
    while (a < n)
        a += tape->unit;

    data = realloc(tape->data, a);
    if (!data)
        return TM_TAPE_ENOMEM;
    tape->data = data;
    tape->asize = a;
    return TM_TAPE_OK;
}

static struct tm_machine_state *
t_machine_current_state(struct tm_machine *machine)
{
    return t_machine_state_get(machine, machine->curr_state);
}

/*!
 *  Creates a new Turing machine.
 */
struct tm_machine *
t_machine_new()
{
    struct tm_machine *machine;
    machine = malloc(sizeof(struct tm_machine));
    machine->curr_state = 0;
    machine->states = NULL;
    machine->state_count = 0;
    return machine;
}

/*!
 *  Destroys the machine and releases associated memory.
 */
void
t_machine_destroy(struct tm_machine *machine)
{
    struct tm_machine_state *s, *t;
    s = machine->states;
    while (s) {
        t = s->next;
        t_machine_destroy_state(s);
        s = t;
    }
    free(machine);
}

/*!
 *  Adds a new state to the provided machine. Returns a pointer to the
 *  tm_machine_state struct for the new state.
 */
struct tm_machine_state *
t_machine_add_state(struct tm_machine *machine)
{
    struct tm_machine_state *s;
    s = malloc(sizeof(struct tm_machine_state));
    s->instrs = NULL;
    s->next = machine->states;
    machine->states = s;
    ++machine->state_count;
    return s;
}

/*!
 *  Inserts \a n states to the provided machine.
 */
void
t_machine_insert_states(struct tm_machine *machine, size_t n)
{
    size_t i;
    for (i = 0; i < n; ++i)
        (void) t_machine_add_state(machine);
}

/*!
 *  Returns a pointer to the tm_machine_state struct corresponding to
 *  position \a at or NULL if no such state exists.
 */
struct tm_machine_state *
t_machine_state_get(struct tm_machine *machine, tm_int at)
{
    tm_int i;
    struct tm_machine_state *state;

    assert(machine && at < machine->state_count);
    i = machine->state_count - 1;
    state = machine->states;
    while (i-- > at && state) {
        state = state->next;
    }
    return state;
}

/*!
 *  Adds a new instruction to the provided machine.
 */
void
t_machine_add_instruction(struct tm_machine *machine,
                          tm_int state_in,
                          tm_int state_out,
                          tm_int symbol_in,
                          tm_int symbol_out,
                          uint8_t dir)
{
    struct tm_machine_state *s;
    struct tm_instruction *instr;

    assert(machine);
    instr = malloc(sizeof(struct tm_instruction));
    instr->symbol_in  = symbol_in;
    instr->state_out  = state_out;
    instr->symbol_out = symbol_out;
    instr->direction  = dir;
    s = t_machine_state_get(machine, state_in);
    instr->next = s->instrs;
    s->instrs = instr;
}

/*!
 *  Creates a new (empty) Turing machine tape.
 */
struct tm_tape *
t_machine_tape_new()
{
    struct tm_tape *tape;
    tape = malloc(sizeof(struct tm_tape));
    if (tape) {
        tape->data  = NULL;
        tape->size  = 0;
        tape->asize = 0;
        tape->unit  = 64;
        tape->p     = 0;
    }
    return tape;
}

/*!
 *  Destroys the passed tape and releases associated memory.
 */
void
t_machine_tape_destroy(struct tm_tape *tape)
{
    if (!tape)
        return;
    free(tape->data);
    free(tape);
}

/*!
 *  Appends the symbol \a symbol to the provided tape.
 */
void
t_machine_tape_append_symbol(struct tm_tape *tape, tm_int symbol)
{
    assert(tape);
    if (tape->size + 1 > tape->asize && t_machine_tape_buffer_grow(tape, tape->size + 1) < 0)
        return;
    tape->data[tape->size] = symbol;
    ++tape->size;
}

/*!
 *  Prepends the symbol \a symbol to the provided tape.
 */
void
t_machine_tape_prepend_symbol(struct tm_tape *tape, tm_int symbol)
{
    assert(tape);
    if (tape->size + 1 > tape->asize && t_machine_tape_buffer_grow(tape, tape->size + 1) < 0)
        return;
    memmove(tape->data + 1, tape->data, tape->size);
    ++tape->size;
    *tape->data = symbol;
}

/*!
 *  Runs the machine with the provided tape as input.
 */
int8_t
t_machine_run(struct tm_machine *machine, struct tm_tape *tape)
{
    struct tm_instruction *instr;
    tm_int symbol;

    assert(machine && machine->state_count && tape);

    tape->p = 0;
    while (machine->curr_state < machine->state_count - 1) {
        instr = t_machine_current_state(machine)->instrs;

        if (tape->p < 0) {
            t_machine_tape_prepend_symbol(tape, 0);
            tape->p = 0;
        }
        if (tape->p >= (int) tape->size)
            t_machine_tape_append_symbol(tape, 0);

        symbol = tape->data[tape->p];
        while (instr && instr->symbol_in != symbol)
            instr = instr->next;

        t_machine_dump_tape(tape);
        if (!instr) {
            fprintf(stderr, "t_machine_run error: no instruction\n");
            return -1;
        }

        machine->curr_state = instr->state_out;
        tape->data[tape->p] = instr->symbol_out;
        tape->p += TM_LEFT == instr->direction ? -1 : 1;
        /* (void) getchar(); */
    }

    t_machine_dump_tape(tape);
    return 0;
}

/*!
 *  Prints out the tape to stdout.
 */
void
t_machine_dump_tape(struct tm_tape *tape)
{
    unsigned int i;
    tm_int symbol;

    assert(tape);
    for (i = 0; i < tape->size; ++i) {
        symbol = tape->data[i];
        printf("| %i ", symbol);
    }
    printf("|\n");
    while (i--)
        printf(i == (tape->size - tape->p - 1) ? "--^-" : "----");
    printf("-\n");
}
