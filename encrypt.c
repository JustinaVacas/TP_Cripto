#include "include/encrypt.h"

static Status generate_key_iv(uchar_ptr password, uchar_ptr key, uchar_ptr iv, char * enc) {
    const EVP_CIPHER * cipher = EVP_get_cipherbyname(enc);
    if (cipher == NULL) {
        printf("ERROR: Error getting cipher %s\n\n", enc);
        return e_failure;
    }
    const EVP_MD * digest = EVP_get_digestbyname("sha256");
    if (digest == NULL) {
        printf("ERROR: Error getting digest sha256\n\n");
        return e_failure;
    }
    if (!(EVP_BytesToKey(cipher, digest, NULL, password, (int) strlen((char *) password), 1, key, iv))) {
        printf("ERROR: Error generating key iv\n\n");
        return e_failure;
    }
    return e_success;
}

static char * get_type(EncType enc){
    char * type = NULL;
    switch (enc) {
        case AES_128:
            type = "aes-128";
            break;
        case AES_192:
            type = "aes-192";
            break;
        case AES_256:
            type = "aes-256";
            break;
        case DES:
            type = "des";
            break;
        default:
            printf("ERROR: Nonexistent encoding type %d\n", enc);
            return NULL;
    }
    return type;
}

static char * get_type_and_mode(EncType enc, EncMode mode){
    char * type = get_type(enc);
    if (type == NULL){
        return NULL;
    }
    char * mode_st = NULL;

    switch (mode) {
        case CBC:
            mode_st = "cbc";
            break;
        case ECB:
            mode_st = "ecb";
            break;
        case OFB:
            mode_st = "ofb";
            break;
        case CFB:
            mode_st = "cfb";
            break;
        default:
            printf("ERROR: Nonexistent encoding mode %d\n", mode);
            return NULL;
    }
    char * aux = malloc(strlen(type) + strlen(mode_st) + 2);
    strcpy(aux, type);
    strcat(aux, "-");
    strcat(aux, mode_st);
    return aux;
}

Status encrypt(uchar_ptr plaintext, int plaintext_len, uchar_ptr ciphertext, EncType enc, EncMode mode, uchar_ptr password, uint * size) {
    OpenSSL_add_all_algorithms();
    char * evp_cipher_name = get_type_and_mode(enc, mode);

    uchar key[EVP_MAX_KEY_LENGTH];
    uchar iv[EVP_MAX_IV_LENGTH];
    if (generate_key_iv(password, key, iv, evp_cipher_name)) {
        printf("INFO: Key and IV generated\n");
    }else{
        printf("ERROR: Key and IV generation failed\n\n");
        return e_failure;
    }

    EVP_CIPHER_CTX *ctx;
    int len;
    int ciphertext_len;

    /* Create and initialise the context */
    if ((ctx = EVP_CIPHER_CTX_new())) {
        printf("INFO: Cipher generated\n");
    }else{
        printf("ERROR: Cipher generation failed\n\n");
        return e_failure;
    }

    /*
     * Initialise the encryption operation.
     */
    if (EVP_EncryptInit_ex(ctx, EVP_get_cipherbyname(evp_cipher_name), NULL, key, iv) == 1){
        printf("INFO: Encryption operation initialised\n");
    }else{
        printf("ERROR: Encryption operation failed\n\n");
        return e_failure;
    }

    /*
     * Provide the message to be encrypted, and obtain the encrypted output.
     * EVP_EncryptUpdate can be called multiple times if necessary
     */
    if(EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len) == 1){
        printf("INFO: Encryption operation started\n");
    }else{
        printf("ERROR: Encryption operation failed\n\n");
        return e_failure;
    }
    ciphertext_len = len;

    /*
     * Finalise the encryption. Further ciphertext bytes may be written at
     * this stage.
     */
    if(EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) == 1){
        printf("INFO: Encryption operation finalised\n");
    }else{
        printf("ERROR: Encryption operation failed\n\n");
        return e_failure;
    }
    ciphertext_len += len;

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);
    free(evp_cipher_name);

    /* Assign text size */
    *size = (uint) ciphertext_len;

    return e_success;
}

Status decrypt(uchar_ptr ciphertext, int ciphertext_size, uchar_ptr plaintext, EncType enc, EncMode mode, uchar_ptr password, uint * size){
    OpenSSL_add_all_algorithms();

    EVP_CIPHER_CTX *ctx;
    int len;
    int plaintext_len;

    char * evp_cipher_name = get_type_and_mode(enc, mode);

    uchar key[EVP_MAX_KEY_LENGTH];
    uchar iv[EVP_MAX_IV_LENGTH];
    if (generate_key_iv(password, key, iv, evp_cipher_name)) {
        printf("INFO: Key and IV generated\n");
    }else{
        printf("ERROR: Key and IV generation failed\n\n");
        return e_failure;
    }

    /* Create and initialise the context */
    if ((ctx = EVP_CIPHER_CTX_new())) {
        printf("INFO: Cipher %s generated\n", evp_cipher_name);
    }else{
        printf("ERROR: Cipher generation failed\n\n");
        return e_failure;
    }

    /*
     * Initialise the decryption operation.
     */
    if (EVP_DecryptInit_ex(ctx, EVP_get_cipherbyname(evp_cipher_name), NULL, key, iv) == 1){
        printf("INFO: Decryption operation initialised\n");
    }else{
        printf("ERROR: Decryption operation failed\n\n");
        return e_failure;
    }

    /*
     * Provide the message to be decrypted, and obtain the plaintext output.
     * EVP_DecryptUpdate can be called multiple times if necessary.
     */
    if(EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_size) == 1){
        printf("INFO: Decryption operation started\n");
    }else{
        printf("ERROR: Decryption operation failed\n\n");
        return e_failure;
    }
    plaintext_len = len;

    /*
     * Finalise the decryption. Further plaintext bytes may be written at
     * this stage.
     */
    if(EVP_DecryptFinal_ex(ctx, plaintext + len, &len) == 1){
        printf("INFO: Decryption operation finalised\n");
    }else{
        printf("ERROR: Decryption operation failed\n\n");
        return e_failure;
    }
    plaintext_len += len;

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);
    free(evp_cipher_name);

    /* Assign text size */
    *size = (uint) plaintext_len;

    return e_success;
}
