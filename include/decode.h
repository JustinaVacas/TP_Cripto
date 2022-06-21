#ifndef DECODE_H
#define DECODE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "encode.h"
#include "encrypt.h"
#include "types.h"

/* Perform decoding */
Status do_decoding(EncodeInfo *encInfo);

#endif //DECODE_H
