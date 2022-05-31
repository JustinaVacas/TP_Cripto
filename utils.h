#ifndef UTILS_H
#define UTILS_H

struct args {
    bool embed;
    bool extract;
    char * in;
    char * p;
    char * out;
    char * pass;
    steg_t steg;
    enc_t enc;
    mode_t mode;
};

typedef enum steg {
    LSB1,
    LSB4,
    LSBI
} steg_t;

typedef enum enc {
    AES_128,
    AES_192,
    AES_256,
    DES,
} enc_t;

typedef enum mode {
    ECB,
    CFB,
    OFB,
    CBC,
} mode_t;

#endif