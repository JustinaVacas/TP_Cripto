#ifndef TP_CRIPTO_ENCRYPT_H
#define TP_CRIPTO_ENCRYPT_H

#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include "types.h"

typedef enum EncType{
    AES_128,
    AES_192,
    AES_256,
    DES
} EncType;

typedef enum EncMode{
    ECB,
    CFB,
    OFB,
    CBC
} EncMode;

Status encrypt(uchar_ptr plaintext, int plaintext_len, uchar_ptr ciphertext, EncType enc, EncMode mode, uchar_ptr password, uint * size);
Status decrypt(uchar_ptr ciphertext, int ciphertext_size, uchar_ptr plaintext, EncType enc, EncMode mode, uchar_ptr password, uint * size);

#endif //TP_CRIPTO_ENCRYPT_H
