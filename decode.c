#include "include/decode.h"

extern uint password_flag;
static uchar (*chosenStegoAlgorithm)(FILE *); // LSB1, LSB4, LSBI
static int inverted_flags[4]; // 00, 01, 10, 11

static Status get_inverted_flags(FILE * input) {
    uchar scan_char;//Read and store each byte
    for (uint i = 0; i < 4; i++)//4 times inner iteration
    {
        uchar ch = 0;
        fread(&scan_char, sizeof(scan_char), 1, input);
        if (ferror(input))//Error handling
        {
            printf("ERROR: Error while reading from source file\n\n");
            exit(e_success);
        }
        scan_char &= 01;
        ch |= scan_char;
        inverted_flags[i] = (int) ch;
    }
    return e_success;
}

static uchar lsb_1(FILE * input){
    uchar scan_char;//Read and store each character
    uchar ch = 0;//To store every obtained byte
    for (uint i = 0; i < 8; i++)//8 times inner iteration
    {
        //Read and store each bit
        fread(&scan_char, sizeof(scan_char), 1, input);
        if (ferror(input))//Error handling
        {
            printf("ERROR: Error while reading from source file\n\n");
            exit(e_success);
        }
        scan_char &= 01;//Obtain the least significant bit
        ch <<= 1;//Left shift by 1 bit to store obtained least significant bit
        ch |= scan_char;//Store the obtained least significant bit
    }
    return ch;
}

static uchar lsb_4(FILE * input){
    uchar scan_char;//Read and store each character
    uchar ch = 0;//To store every obtained byte
    for (uint i = 0; i < 2; i++)//4 times inner iteration
    {
        //Read and store each bit
        fread(&scan_char, sizeof(scan_char), 1, input);
        if (ferror(input))//Error handling
        {
            printf("ERROR: Error while reading from source file\n\n");
            exit(e_success);
        }
        scan_char &= 0x0F;//Obtain the 4 least significant bits
        ch <<= 4;//Left shift by 4 bits to store obtained 4 least significant bits
        ch |= scan_char;//Store the obtained 4 least significant bits
    }
    return ch;
}

static uchar lsb_i(FILE * input){
    uchar scan_char;//Read and store each character
    uchar ch = 0;//To store every obtained byte
    for (uint i = 0; i < 8; i++)//8 times inner iteration
    {
        //Read and store each bit
        fread(&scan_char, sizeof(scan_char), 1, input);
        if (ferror(input))//Error handling
        {
            printf("ERROR: Error while reading from source file\n\n");
            exit(e_success);
        }
        uchar lsb = GET_nLSB(scan_char, 1);//Obtain the least significant bit
        uchar second_lsb = GET_nLSB(scan_char, 2);
        uchar third_lsb = GET_nLSB(scan_char, 3);
        int index = 2 * third_lsb + second_lsb;
        if (inverted_flags[index]){
            lsb = lsb ^ (1<<0);
        }
        ch <<= 1;//Left shift by 1 bit to store obtained least significant bit
        ch |= lsb;//Store the obtained least significant bit
    }
    return ch;
}

static uchar_ptr decode_extension(FILE * ptr) {
    //Pointer to hold the heap memory of given size
    uchar_ptr decoded_ext = (uchar_ptr) malloc(MAX_EXTENSION_SIZE * sizeof(uchar));
    if (decoded_ext == NULL)
    {
        printf("ERROR: Unable to allocate dynamic memory.\n\n");
        exit(e_success);
    }
    uint j;//Outer iterator
    for (j = 0; j < MAX_EXTENSION_SIZE ; j++)//Iterate till given string size
    {
        decoded_ext[j] = chosenStegoAlgorithm(ptr);
        if (decoded_ext[j] == '\0'){
            break;
        }
    }
    if (decoded_ext[0] == '.'){
        printf("INFO: Successfully decoded the dot in the secret filename.\n");
    }else{
        printf("ERROR: Dot for file extension not found %s.\n\n", decoded_ext);
        return NULL;
    }
    return decoded_ext;//Return obtained string
    //Free the heap memory at caller side after executing this function
}

static uint decode_int_size_expression(FILE * ptr)
{
    uint decoded_int = 0;//To store decoded integer value
    for (uint j = 0; j < INT_SIZE; j++)//Integer size outer iterations
    {
        uchar scan_char = chosenStegoAlgorithm(ptr);//Read and store each character
        decoded_int <<= 8; // move 1 byte
        decoded_int |= (uint) scan_char; //Store the obtained byte
    }
    return decoded_int;//Return obtained integer
}

static uchar_ptr decode_file_data(uint f_size, FILE * ptr)
{
    //Pointer to hold the heap memory of file size
    uchar_ptr file_data = (uchar_ptr) malloc(f_size * sizeof(uchar));
    if (file_data == NULL)
    {
        printf("ERROR: Unable to allocate dynamic memory.\n\n");
        exit(e_success);
    }
    for (uint j = 0; j < f_size; j++)//File size outer iterations
    {
        file_data[j] = chosenStegoAlgorithm(ptr);
    }
    return file_data;
}


Status do_decoding(EncodeInfo * encInfo)
{
    switch (encInfo->stego_mode) {
        case LSB1:
            chosenStegoAlgorithm = lsb_1;
            break;
        case LSB4:
            chosenStegoAlgorithm = lsb_4;
            break;
        case LSBI:
            chosenStegoAlgorithm = lsb_i;
            get_inverted_flags(encInfo->fptr_stego_image);
            printf("INFO: Successfully decoded inverted flags: 00 = %d | 01 = %d | 10 = %d | 11 = %d\n", inverted_flags[0], inverted_flags[1], inverted_flags[2], inverted_flags[3]);
            break;
    }

    uchar_ptr file_data = NULL;

    if(password_flag){
        printf("INFO: Password detected\n");
        //Decode encrypted data size
        printf("INFO: Decoding encrypted data size\n");
        uint enc_size = decode_int_size_expression(encInfo->fptr_stego_image);
        if (enc_size)
        {
            printf("INFO: Successfully decoded the encrypted data size. It is %u bytes\n", enc_size);
        }
        else
        {
            printf("ERROR: Failed to decode the secret data size\n\n");
            return e_failure;
        }

        //Decode file encrypted data
        printf("INFO: Decoding and storing the encrypted data\n");
        file_data = decode_file_data(enc_size, encInfo->fptr_stego_image);
        printf("INFO: Successfully decoded the encrypted data.\n");

        // Decrypt decoded data
        printf("INFO: Decrypting data\n");
        uchar_ptr plaintext = malloc(enc_size * sizeof(uchar));
        if(plaintext == NULL){
            printf("ERROR: Unable to allocate dynamic memory.\n\n");
            exit(e_success);
        }
        uint plaintext_len = 0;
        if(decrypt(file_data, (int) enc_size, plaintext, encInfo->enc, encInfo->enc_mode, encInfo->password, &plaintext_len)){
            printf("INFO: Successfully decrypted the secret data.\n");
        }else
        {
            printf("ERROR: Failed to decrypt secret data\n\n");
            return e_failure;
        }

        // First 4 bytes of encrypted data is secret file size
        encInfo->secret_fsize = 0;
        for (uint j = 0; j < INT_SIZE; j++)//Integer size outer iterations
        {
            uchar scan_char = plaintext[j];//Read and store each character
            encInfo->secret_fsize <<= 8; // move 1 byte
            encInfo->secret_fsize |= (uint) scan_char; //Store the obtained byte
        }
        printf("INFO: Successfully decoded the secret data size. It is %u bytes\n", encInfo->secret_fsize);
        plaintext += 4;

        //Decode File extension
        printf("INFO: Decoding secret file extension\n");
        uchar_ptr extension = plaintext + encInfo->secret_fsize;
        if (*extension == '.') {
            strcpy((char *) encInfo->secret_fext, (const char *) extension);
            printf("INFO: Successfully decoded the secret file extension %s\n", encInfo->secret_fext);
        } else {
            printf("ERROR: Failed to decode the secret file extension.\n\n");
            return e_failure;
        }
        // Add secret file extension to secret file name
        strcat((char *) encInfo->secret_fname, (const char *) encInfo->secret_fext);

        //Let's open the decode file for writing the secret data
        printf("INFO: Opening file %s for writing the secret data\n", encInfo->secret_fname);
        if ((encInfo->fptr_secret = fopen((const char *) encInfo->secret_fname, "wb")) != NULL) {
            printf("INFO: Successfully opened the file.\n");
        } else {
            printf("ERROR: Unable to open the file %s. This file may not be present in the current project directory.\n\n",
                   encInfo->secret_fname);
            return e_failure;
        }

        //Write obtained heap data onto decoded file
        fwrite(plaintext, encInfo->secret_fsize, 1, encInfo->fptr_secret);
        if (ferror(encInfo->fptr_secret))//Error handling
        {
            printf("ERROR: Error while writing into file %s\n\n", encInfo->secret_fname);
            return e_failure;
        }
        printf("INFO: Decoded data has been successfully written onto file %s\n", encInfo->secret_fname);

        // Free the data
        plaintext -= 4;
        free(plaintext);

    }else {
        /*
        // Decode without encryption
         */

        printf("INFO: No password detected\n");
        //Decode secret data size
        printf("INFO: Decoding secret data size\n");
        encInfo->secret_fsize = decode_int_size_expression(encInfo->fptr_stego_image);
        if (encInfo->secret_fsize) {
            printf("INFO: Successfully decoded the secret data size. It is %u bytes\n", encInfo->secret_fsize);
        } else {
            printf("ERROR: Failed to decode the secret data size\n\n");
            return e_failure;
        }

        //Decode file data
        printf("INFO: Decoding and storing the secret file data\n");
        file_data = decode_file_data(encInfo->secret_fsize, encInfo->fptr_stego_image);
        printf("INFO: Successfully decoded the secret file data.\n");

        //Decode File extension
        printf("INFO: Decoding secret file extension\n");
        uchar_ptr extension = decode_extension(encInfo->fptr_stego_image);
        if (extension != NULL) {
            strcpy((char *) encInfo->secret_fext, (const char *) extension);
            printf("INFO: Successfully decoded the secret file extension %s\n", encInfo->secret_fext);
            free(extension);
        } else {
            printf("ERROR: Failed to decode the secret file extension.\n\n");
            return e_failure;
        }
        // Add secret file extension to secret file name
        strcat((char *) encInfo->secret_fname, (const char *) encInfo->secret_fext);

        //Let's open the decode file for writing the secret data
        printf("INFO: Opening file %s for writing the secret data\n", encInfo->secret_fname);
        if ((encInfo->fptr_secret = fopen((const char *) encInfo->secret_fname, "wb")) != NULL) {
            printf("INFO: Successfully opened the file.\n");
        } else {
            printf("ERROR: Unable to open the file %s. This file may not be present in the current project directory.\n\n",
                   encInfo->secret_fname);
            return e_failure;
        }

        //Write obtained heap data onto decoded file
        fwrite(file_data, encInfo->secret_fsize, 1, encInfo->fptr_secret);
        if (ferror(encInfo->fptr_secret))//Error handling
        {
            printf("ERROR: Error while writing into file %s\n\n", encInfo->secret_fname);
            return e_failure;
        }
        printf("INFO: Decoded data has been successfully written onto file %s\n", encInfo->secret_fname);
    }
    free(file_data);//Free allocated heap memory
    return e_success;//No error found
}

