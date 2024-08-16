#include "jumble.h"
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Jumbler FSM states and their transitions. */

static const state_t STATES[] = {

    /* The start state is the initial state of the FSM.
     * If an alphabetic character is received, handle the first character of a word.
     * If a non-alphabetic character is received, go handle it.
     */
    [STATE_START] =
        {
            .cur = STATE_START,
            .trans_char = STATE_ALPHA_FIRST,
            .trans_nonchar = STATE_NONALPHA,
        },

    /* The first alphabetical character state handles the first character of a word by printing it immediately.
     * If an alphabetic character is received, handle the next character of a word.
     * If a non-alphabetic character is received, go handle it.
     */
    [STATE_ALPHA_FIRST] =
        {
            .cur = STATE_ALPHA_FIRST,
            .trans_char = STATE_ALPHA_N,
            .trans_nonchar = STATE_NONALPHA,
        },

    /* The nth alphabetic character state handles the nth character of a word by storing it for jumbling.
     * If an alphabetic character is received, return to this state.
     * If a non-alphabetic character is received, go handle it.
     */
    [STATE_ALPHA_N] =
        {
            .cur = STATE_ALPHA_N,
            .trans_char = STATE_ALPHA_N,
            .trans_nonchar = STATE_JUMBLE,
        },

    /* The non-alphabetic character state handles non-alphabetic characters by printing them immediately.
     * If an alphabetic character is received, go handle the first character of a word.
     * If a non-alphabetic character is received, return to this state.
     */
    [STATE_NONALPHA] =
        {
            .cur = STATE_NONALPHA,
            .trans_char = STATE_ALPHA_FIRST,
            .trans_nonchar = STATE_NONALPHA,
        },

    /* The jumble state handles the end of a word. It jumbles the stored buffer of alphabetic characters comprising the
     * middle of the word that was just read, and then prints them. It then prints the last character of the word that
     * was just read, and its trailing non-alphabetic character which resulted in the trigger of the word's end.
     * If an alphabetic character is received, go handle the first character of a word.
     * If a non-alphabetic character is received, go handle it.
     */
    [STATE_JUMBLE] =
        {
            .cur = STATE_JUMBLE,
            .trans_char = STATE_ALPHA_FIRST,
            .trans_nonchar = STATE_NONALPHA,
        },
};

/* Initialize the FSM.
 * @param fsm The FSM structure to be initialized.
 * @param source The source location to read characters from.
 * @param output The output location to write jumbled characters to.
 */
void jumble_fsm_init(jumble_fsm_t *fsm, FILE *source, FILE *output) {
    fsm->source = source;
    fsm->output = output;
    fsm->state = STATES[STATE_START];
    fsm->bufidx = 0;
    memset(fsm->buf, 0, sizeof(fsm->buf));
}

/* Executes the logic associated with the FSM's current state.
 * @param fsm The FSM to execute the state logic for.
 */
static int jumble_fsm_execute(jumble_fsm_t *fsm) {
    switch (fsm->state.cur) {

    case STATE_START:
        break;

    case STATE_NONALPHA:
        if (fputc(fsm->lastchar, fsm->output) == EOF) return errno;
        break;

    case STATE_ALPHA_FIRST:
        if (fputc(fsm->lastchar, fsm->output) == EOF) return errno;
        break;

    case STATE_ALPHA_N:
        fsm->buf[fsm->bufidx] = fsm->lastchar;
        fsm->bufidx++;
        break;

    case STATE_JUMBLE:

        /* The `-1` allows the shuffling logic to believe the array is one character shorter than reality. This is
         * because the last character in the array is the last character of the word, which should be kept in its
         * original place (not shuffled).
         */
        jumble(fsm->buf, fsm->bufidx - 1);

        /* Write the shuffled array to output */
        for (size_t i = 0; i < fsm->bufidx; i++) {
            if (fputc(fsm->buf[i], fsm->output) == EOF) return errno;
        }

        /* Print previous white space that triggered the end of word detection. */
        if (fputc(fsm->lastchar, fsm->output) == EOF) return errno;

        /* Reset the index into the buffer to start storing the next word at the beginning. */
        fsm->bufidx = 0;

        break;
    }
    return 0;
}

/* Gets the next character in the input stream.
 * @param fsm The FSM whose next character should be fetched.
 * @return 0 for success, errno on failure.
 */
static int jumble_fsm_nextchar(jumble_fsm_t *fsm) {
    fsm->lastchar = fgetc(fsm->source);
    if (fsm->lastchar == EOF && ferror(fsm->source)) {
        return errno;
    }
    return 0;
}

/* Advances the FSM to its next state depending on the last input.
 * @param fsm The FSM whose state to transition.
 */
static void jumble_fsm_nextstate(jumble_fsm_t *fsm) {
    if (isalpha(fsm->lastchar)) {
        /* An alphabetic character was the last input. */
        fsm->state = STATES[fsm->state.trans_char];
    } else {
        /* A non-alphabetic character was the last input. */
        fsm->state = STATES[fsm->state.trans_nonchar];
    }
}

/* Run the FSM to completion, generating output and consuming the input source file.
 * @param fsm The FSM structure containing the data to be processed.
 * @return 0 for successful execution, an errno otherwise.
 */
int jumble_fsm_run(jumble_fsm_t *fsm) {

    int err;

    while (!feof(fsm->source) && !ferror(fsm->source)) {

        /* Execute state logic */
        err = jumble_fsm_execute(fsm);
        if (err) return err;

        /* Get the next character. */
        err = jumble_fsm_nextchar(fsm);
        if (err) return err;

        /* Perform the state transition based on the input to the FSM. */
        jumble_fsm_nextstate(fsm);
    }

    /* The loop has now exited, which means one of two things: */

    /* There was an error. */
    if (ferror(fsm->source)) {
        return errno;
    }

    /* The stream finished; success! */
    if (feof(fsm->source)) {
        return 0;
    }

    /* If we got here somehow, something went horribly wrong. */
    assert(false);
    return EIO;
}

/* Randomly shuffles an array of characters.
 * @param arr The array containing the characters to be shuffled.
 * @param n The number of characters in the array.
 */
void jumble(char *arr, size_t n) {

    /* Nothing to shuffle. */

    if (n <= 1) return;

    /* Shuffle the characters. */

    size_t i;
    for (i = 0; i < n; i++) {
        size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
        char t = arr[j];
        arr[j] = arr[i];
        arr[i] = t;
    }
}
