#ifndef _JUMBLE_H_
#define _JUMBLE_H_

#include <stdbool.h>
#include <stdio.h>

#define JUMBLE_BUFSIZ 1024

/* Defines the different states the jumbler FSM has. */

typedef enum {
    STATE_START,       /* Initial/start state */
    STATE_NONALPHA,    /* State to handle a non-alphabetic character */
    STATE_ALPHA_FIRST, /* State to handle the first character in a word */
    STATE_ALPHA_N,     /* State to handle the nth (not first) character in a word */
    STATE_JUMBLE,      /* State to jumble a word that has ended */
} state_e;

/* Defines a state and its transition states. */

typedef struct {
    state_e cur;           /* Current state */
    state_e trans_char;    /* Next state if an alphabetic character is received */
    state_e trans_nonchar; /* Next state if non-alphabetic character is received */
} state_t;

typedef struct jumble_fsm_t {
    state_t state;           /* The current state of the FSM */
    FILE *source;            /* Source of characters to jumble */
    FILE *output;            /* Output location for jumbled characters */
    char lastchar;           /* Last received character */
    char buf[JUMBLE_BUFSIZ]; /* Buffer for storing characters to be jumbled */
    size_t bufidx;           /* Current index into the buffer to store next character */
} jumble_fsm_t;

/* FSM logic. */

void jumble_fsm_init(jumble_fsm_t *fsm, FILE *source, FILE *output);
int jumble_fsm_run(jumble_fsm_t *fsm);

void jumble(char *arr, size_t n);

#endif // _JUMBLE_H_
