#ifndef ENCODE_H
#define ENCODE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "encrypt.h"

#define MAX_FILENAME_SIZE   100
#define MAX_EXTENSION_SIZE  5
#define MAX_PASSWORD_LEN    50
#define INT_SIZE            sizeof(int)
#define CHAR_SIZE           sizeof(char)
#define SIZE_OF_HEADER      54
#define BLOCK_SIZE          150
#define GET_nLSB(byte, n) ((byte >> (n-1) & 1))

typedef enum StegoMode{
    LSB1,
    LSB4,
    LSBI
} StegoMode;

/* 
 * Structure to store information required for
 * encoding & decoding
 */

typedef struct EncodeInfo
{
    /* Normal Image info */
    FILE *fptr_src_image;
    uchar src_image_fname[MAX_FILENAME_SIZE];
    uint src_image_size;
    /* Secret File Info */
    FILE *fptr_secret;
    uchar secret_fname[MAX_FILENAME_SIZE];
    uchar secret_fext[MAX_EXTENSION_SIZE + CHAR_SIZE];
    uint secret_fsize;
    uint secret_extn_len;
    /* Stego Image Info */
    FILE *fptr_stego_image;
    uchar stego_image_fname[MAX_FILENAME_SIZE];
    uint stego_fsize;
    uint stego_mode;
    /*Optional*/
    uint enc;
    uint enc_mode;
    uchar password[MAX_PASSWORD_LEN];
} EncodeInfo;

/* Function prototypes */

/* Copy bmp image header */
Status copy_bmp_header(FILE * fptr_src_image, FILE * fptr_dest_image);

/* Check compression type is set to 0 in bmp file */
Status check_compression(FILE * fptr);

/* Get File pointers for i/p and o/p files */
Status open_files(EncodeInfo *encInfo);

/* Read and validate Encode args from argv */
Status read_and_validate_bmp_format(char * arg);

/* Read, validate and extract secret file extension */
Status read_and_validate_extn(uchar_ptr sec_file_name_holder, EncodeInfo *encInfo);

/* check capacity */
Status check_capacity(EncodeInfo *encInfo);

/* Get image size */
uint get_image_size_for_bmp(FILE *fptr_image);

/* Get file size */
uint get_file_size(FILE *fptr);

/* Perform encoding */
Status do_encoding(EncodeInfo *encInfo);

#endif
