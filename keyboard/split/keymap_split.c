#include "keymap_common.h"

const uint8_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /* Normal (GRV replaced by NUBS for ISO keyboard) */
    KEYMAP2( ESC,  F1,   F2,   F3,   F4,   F5,   F6,   F7,   F8,   F9,   F10,  F11,  F12,  \
             NUBS, 1,    2,    3,    4,    5,    6,    7,    8,    9,    0,    MINS, EQL,  \
             TAB,  Q,    W,    E,    R,    T,    Y,    U,    I,    O,    P,    LBRC, RBRC, \
             ENT,  A,    S,    D,    F,    G,    H,    J,    K,    L,    SCLN, QUOT, BSLS, \
             LSFT, Z,    X,    C,    V,    B,    N,    M,    COMM, DOT,  SLSH, RSFT, ENT,  \
             LCTL, LALT, LGUI, APFN, SPC,  BSPC, BSPC, SPC,  PAUS, RGUI, RALT, RCTL,       \
             LEFT, DOWN, UP,   RGHT, FN0,              FN0,        LEFT, DOWN, UP,   RGHT),
    /* Swapped NUBS and GRV (In linux) */
    KEYMAP2( TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, \
             GRV,  TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, \
             TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, \
             TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, \
             TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, \
             TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS,       \
             TRNS, TRNS, TRNS, TRNS, FN1,              FN1,        TRNS, TRNS, TRNS, TRNS),
    /* Pg/Pd for Normal */
    KEYMAP2( TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, \
             GRV,  TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, \
             TRNS, TRNS, TRNS, UP,   TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, \
             CAPS, TRNS, LEFT, DOWN, RGHT, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, \
             TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, \
             TRNS, TRNS, TRNS, TRNS, TRNS, DEL,  DEL,  TRNS, TRNS, TRNS, TRNS, TRNS,       \
             HOME, PGDN, PGUP, END,  TRNS,             TRNS,       HOME, PGDN, PGUP, END),
    /* Pg/Pd for Swapped */
    KEYMAP2( TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, \
             NUBS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, \
             TRNS, TRNS, TRNS, UP,   TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, \
             CAPS, TRNS, LEFT, DOWN, RGHT, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, \
             TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, TRNS, \
             TRNS, TRNS, TRNS, TRNS, TRNS, DEL,  DEL,  TRNS, TRNS, TRNS, TRNS, TRNS,       \
             HOME, PGDN, PGUP, END,  TRNS,             TRNS,       HOME, PGDN, PGUP, END)
};

const action_t PROGMEM fn_actions[] = {
    [0] = ACTION_LAYER_MOMENTARY(2),
    [1] = ACTION_LAYER_MOMENTARY(3),
};
