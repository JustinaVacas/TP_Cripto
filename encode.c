#include "include/encode.h"

extern uint password_flag;
static Status (*chosenStegoAlgorithm)(uchar, FILE *, FILE *); // LSB1, LSB4, LSBI
static int inverted_flags[4]; // 00, 01, 10, 11
static int inverted_bits[4][2]; // 00[same][diff], 01[same][diff], 10[same][diff], 11[same][diff]

/* Function Definitions */
Status copy_bmp_header(FILE * fptr_src_image, FILE * fptr_dest_image)
{
	uchar_ptr img_header = (uchar_ptr) malloc(SIZE_OF_HEADER * sizeof(uchar));
	if (img_header == NULL)//Error handling for malloc
	{
		printf("ERROR: Unable to allocate dynamic memory.\n\n");
		exit(e_success);
	}
	//Read bytes of the size of raster data from source file
	fread(img_header, SIZE_OF_HEADER, 1, fptr_src_image);
	if (ferror(fptr_src_image))//Error handling while reading from file
	{
		printf("ERROR: Error in reading source image file.\n\n");
		return e_failure;
	}

	//Write data obtained in heap onto the destination file
	fwrite(img_header, SIZE_OF_HEADER, 1, fptr_dest_image);
	if (ferror(fptr_dest_image))//Error handling while writing onto destination file
	{
		printf("ERROR: Error in writing onto destination image file.\n\n");
		return e_failure;
	}
	free(img_header);//Free dynamically allocated block of memory
	return e_success;//No error found
}

Status check_compression(FILE * fptr){
	uint comp;
	fseek(fptr, 0x1eL, SEEK_SET);
	fread(&comp, sizeof(uint), 1, fptr);
	if (comp)
	{
		printf("ERROR: Compression type %u is set on source bmp file.\n", comp);
		return(e_failure);
	}	
	rewind(fptr);
	return(e_success);
}

uint get_image_size_for_bmp(FILE *fptr_image)
{
		uint img_size;//Return value to be read from image file
		//Seek to 34th byte to get image data size from the '.bmp' image file
		fseek(fptr_image, 34L, SEEK_SET);
		//Read the width (an unsigned integer)
		fread(&img_size, sizeof(img_size), 1, fptr_image);
		if (ferror(fptr_image))
		{
				printf("ERROR: Error while reading from the image file.\n\n");
				exit(e_success);
		}
		//Return image capacity
		return img_size;
}

uint get_file_size(FILE *fptr)//Returns file size including EOF byte
{
	//Seek to the end of file
	fseek(fptr, 0L, SEEK_END);
	uint aux = (uint) ftell(fptr);
	rewind(fptr);
	return aux;//Return file index value
}

Status check_capacity(EncodeInfo *encInfo){
	uint data, max_capacity; // Data to store and maximum capacity of bmp file in bits

	if (password_flag)
	{
		data = (4 + 4 + encInfo->secret_fsize + encInfo->secret_extn_len + CHAR_SIZE) * 8; // Size + Enc(Size + Data + Ext) + '\0'
	}
	else {
		data = (4 + encInfo->secret_fsize + encInfo->secret_extn_len + CHAR_SIZE) * 8; // Size + Data + Ext + '\0'
	}

	switch (encInfo->stego_mode)
	{
	case LSB1:
		max_capacity = encInfo->src_image_size; // 1 bit per byte
		break;
	case LSB4:
		max_capacity = 4 * encInfo->src_image_size; // 4 bits per byte
		break;
	case LSBI:
		data = 4 + data; // 4 extra bits for LSBI inverted flags storage
		max_capacity = encInfo->src_image_size; // 1 bit per byte
		break;
	default:
		printf("ERROR: Wrong stego method saved when parsing.\n\n");
		exit(e_success);
	}

	if (data > max_capacity)
	{
		printf("ERROR: Data size to hide (%d bytes) is greater than maximum capacity of bmp file (%d bytes).\n\n", data, max_capacity);
		return (e_failure);
	}

	return(e_success);
}

Status open_files(EncodeInfo *encInfo)
{
	//Open source image file with proper error handling
	if ((encInfo->fptr_src_image = fopen((const char*)encInfo->src_image_fname, "rb")) == NULL)
	{
			perror("fopen");
			fprintf(stderr, "ERROR: Unable to open file %s. This file may not be present in the current project directory.\n\n", encInfo->src_image_fname);
			return e_failure;
	}
	//Open secret file with proper error handling
	if ((encInfo->fptr_secret = fopen((const char*)encInfo->secret_fname, "rb")) == NULL)
	{
			perror("fopen");
			fprintf(stderr, "ERROR: Unable to open file %s. This file may not be present in the current project directory.\n\n", encInfo->secret_fname);
			return e_failure;
	}
	//Open output image file with proper error handling
	if ((encInfo->fptr_stego_image = fopen((const char*)encInfo->stego_image_fname, "wb")) == NULL)
	{
			perror("fopen");
			fprintf(stderr, "ERROR: Unable to open file %s. This file may not be present in the current project directory.\n\n", encInfo->stego_image_fname);
			return e_failure;
	}
	return e_success;//No error found
}

Status read_and_validate_bmp_format(char * arg)
{
	//Pointer to hold address of '.bmp' part from given argument
	const char* bmp_holder = strstr(arg, ".bmp");
	if(bmp_holder)//Error handling
	{//If '.bmp' part is found, then check if string exactly ends with '.bmp'
			return (!strcmp(bmp_holder, ".bmp")) ? e_success : e_failure;
	}
	return e_failure;//NULL address which means '.bmp' part is not found
}

Status read_and_validate_extn(uchar_ptr sec_file_name_holder, EncodeInfo *encInfo)
{
	//Pointer to hold the heap memory of the size of filename including '\0' character
	uchar_ptr sec = (uchar_ptr) malloc(strlen((const char*)sec_file_name_holder) + 1);
	if (sec == NULL)
	{
			printf("ERROR: Unable to allocate dynamic memory.\n\n");
			return e_failure;
	}
	strcpy((char*)sec, (const char*)sec_file_name_holder);//Store the filename inside allocated heap
	uint secret_filename_len = strlen((const char*)sec);//Get length of filename
	char* ext = strtok((char*)sec, ".");//Get part of string before dot
	//If there is no dot in the filename, length of string remains the same
	if (strlen(ext) == secret_filename_len) {
			printf("ERROR: There is no dot in the given filename.\n");
			return e_failure;
	}
	//Extract the extension of secret file (i.e. part of string after dot)
	ext = strtok(NULL, ".");
	strcpy((char*)encInfo->secret_fext, ".");
	strcat((char*)encInfo->secret_fext, (const char*)ext);//Store the extracted extension
	printf("INFO: Secret file extension = %s.\n", encInfo->secret_fext);
	//Get and store length of secret extension
	encInfo->secret_extn_len = strlen((const char*)encInfo->secret_fext);
	//Validate extension size
	if (encInfo->secret_extn_len > MAX_EXTENSION_SIZE) {
			printf("ERROR: file extension should not exceed 4 characters\n");
			return e_failure;
	}
	free(sec);//Free the allocated block of memory
	return e_success;//No errors found
}

static void size_to_bytes(uchar_ptr bytes, uint size){
    bytes[0] = (size >> 24) & 0xFF;
    bytes[1] = (size >> 16) & 0xFF;
    bytes[2] = (size >> 8) & 0xFF;
    bytes[3] = size & 0xFF;
}

static Status lsb_1(uchar ch, FILE * source, FILE * dest){
    uchar scan_char, ch_bit;//Read and store each character
    for (int i = 8; i > 0; i--)//8 times inner iteration
    {
        //Read and store each bit
        fread(&scan_char, sizeof(scan_char), 1, source);
        if (ferror(source))//Error handling
        {
            printf("ERROR: Error while reading from source file\n\n");
            return e_failure;
        }

        ch_bit = GET_nLSB(ch, i);//Char bit to stego
        scan_char &= 0xFE;//Clear the least significant bit of fetched character
        scan_char |= ch_bit;//Set the least significant bit

        //Write the obtained byte onto output file
        fwrite(&scan_char, sizeof(scan_char), 1, dest);
        if (ferror(dest))//Error handling
        {
            printf("ERROR: Error while writing onto output image file.\n\n");
            return e_failure;
        }
    }
    return e_success;
}

static Status lsb_4(uchar ch, FILE * source, FILE * dest){
    uchar scan_char, ch_bit;
    for (int i = 2; i > 0; i--)//2 times inner iteration
    {
        //Read and store each bit
        fread(&scan_char, sizeof(scan_char), 1, source);
        if (ferror(source))//Error handling
        {
            printf("ERROR: Error while reading from source file\n\n");
            return e_failure;
        }

        if (i == 2){
            ch_bit = ch>>4 & 0x0F;// Right shift by 4 bits to get 4 most significant bits
        }
        else{
            ch_bit = ch & 0x0F;// 4 lsb bits
        }
        scan_char &= 0xF0;//Clear the least 4 significant bit of fetched character
        scan_char |= ch_bit;//Set the least significant bit of original byte

        //Write the obtained byte onto output file
        fwrite(&scan_char, sizeof(scan_char), 1, dest);
        if (ferror(dest))//Error handling
        {
            printf("ERROR: Error while writing onto output image file.\n\n");
            return e_failure;
        }
    }
    return e_success;
}

static Status lsbi_scan(uchar ch, FILE * source, FILE * dest){
    uchar scan_char, ch_bit, lsb, second_lsb, third_lsb;//Read and store each character
    for (int i = 8; i > 0; i--)//8 times inner iteration
    {
        //Read and store each bit
        fread(&scan_char, sizeof(scan_char), 1, source);
        if (ferror(source))//Error handling
        {
            printf("ERROR: Error while reading from source file\n\n");
            return e_failure;
        }
        lsb = GET_nLSB(scan_char, 1);//Obtain the least significant bit of original byte
        second_lsb = GET_nLSB(scan_char, 2);
        third_lsb = GET_nLSB(scan_char, 3);

        ch_bit = GET_nLSB(ch, i);//Char bit to stego

        // Check if lsb changed
        int index = 2 * third_lsb + second_lsb;
        inverted_bits[index][lsb^ch_bit]++;//XOR operation to see if lsb changes
    }
    return e_success;
}

static Status set_inverted_flags(FILE * source, FILE * dest) {
    // Analyze and set flags
    for (int i = 0; i < 4; ++i) {
        if (inverted_bits[i][0] < inverted_bits[i][1])
            inverted_flags[i] = 1;
        else
            inverted_flags[i] = 0;
    }
    // Store flags in stego file
    uchar scan_char;//Read and store each byte
    for (uint i = 0; i < 4; i++)//4 times inner iteration
    {
        fread(&scan_char, sizeof(scan_char), 1, source);
        if (ferror(source))//Error handling
        {
            printf("ERROR: Error while reading from source file\n\n");
            exit(e_success);
        }

        scan_char &= 0xFE;//Clear the least significant bit of fetched character
        if (inverted_flags[i]){
            scan_char |= 01;
        }

        //Write the obtained byte onto output file
        fwrite(&scan_char, sizeof(scan_char), 1, dest);
        if (ferror(dest))//Error handling
        {
            printf("ERROR: Error while writing onto output image file.\n\n");
            return e_failure;
        }
    }
    return e_success;
}

static Status lsbi_encode(uchar ch, FILE * source, FILE * dest){
    uchar scan_char, ch_bit, second_lsb, third_lsb;//Read and store each character
    for (int i = 8; i > 0; i--)//8 times inner iteration
    {
        //Read and store each bit
        fread(&scan_char, sizeof(scan_char), 1, source);
        if (ferror(source))//Error handling
        {
            printf("ERROR: Error while reading from source file\n\n");
            return e_failure;
        }
        second_lsb = GET_nLSB(scan_char, 2);
        third_lsb = GET_nLSB(scan_char, 3);

        ch_bit = GET_nLSB(ch, i);//Char bit to stego
        // Check flags for inversion
        int index = 2 * third_lsb + second_lsb;
        if (inverted_flags[index]){
            ch_bit = ch_bit ^ (1<<0); // Invert bit to insert
        }

        scan_char &= 0xFE;//Clear the least significant bit of fetched character
        scan_char |= ch_bit;//Set the least significant bit of fetched character

        //Write the obtained byte onto output file
        fwrite(&scan_char, sizeof(scan_char), 1, dest);
        if (ferror(dest))//Error handling
        {
            printf("ERROR: Error while writing onto output image file.\n\n");
            return e_failure;
        }
    }
    return e_success;
}

static Status encode_data(uchar_ptr data, uint data_len, FILE * source, FILE * dest)
{
    for (uint i = 0; i < data_len; i++)
    {
        if(!chosenStegoAlgorithm(data[i], source, dest)){
            printf("ERROR: Error while encoding data.\n\n");
            return e_failure;
        }
    }
    if (!password_flag){
        // Add EOF at the end
        uchar eof = '\0';
        if(!chosenStegoAlgorithm(eof, source, dest)){
            printf("ERROR: Error while encoding data.\n\n");
            return e_failure;
        }
    }

    return e_success;//No errors found
}

Status do_encoding(EncodeInfo * encInfo)
{
    // Setting all pointers
    rewind(encInfo->fptr_secret);
    fseek(encInfo->fptr_src_image, SIZE_OF_HEADER, SEEK_SET);
    fseek(encInfo->fptr_stego_image, SIZE_OF_HEADER, SEEK_SET);

    // Copying data for steganography
    uchar_ptr data = malloc((INT_SIZE + encInfo->secret_fsize + encInfo->secret_extn_len + CHAR_SIZE) * sizeof(uchar));
    if(data == NULL){
        printf("ERROR: Unable to allocate dynamic memory.\n\n");
        exit(e_success);
    }
    // Copy size
    size_to_bytes(data, encInfo->secret_fsize);
    // Copy data
    fread(data + INT_SIZE, encInfo->secret_fsize, 1, encInfo->fptr_secret);
    // Copy extension
    strcat((char *) data + INT_SIZE + encInfo->secret_fsize, (const char*) encInfo->secret_fext);
    // Data length
    uint data_len = INT_SIZE + encInfo->secret_fsize + encInfo->secret_extn_len;
    // EOF
    data[data_len] = '\0';

    // Stego mode
    int lsbi_flag = 0;
    switch (encInfo->stego_mode) {
        case LSB1:
            chosenStegoAlgorithm = lsb_1;
            break;
        case LSB4:
            chosenStegoAlgorithm = lsb_4;
            break;
        case LSBI:
            chosenStegoAlgorithm = lsbi_scan;
            lsbi_flag = 1;
            memset(inverted_bits, 0, sizeof inverted_bits);
            fseek(encInfo->fptr_src_image, 4, SEEK_CUR);
            fseek(encInfo->fptr_stego_image, 4, SEEK_CUR); // We leave 4 bytes to store inverted flags
            break;
    }
    uchar_ptr old_data, ciphertext;
    //Check if password is given to encrypt data
    if (password_flag)
    {
        //Encode password length
        printf("INFO: Encrypting secret data...\n");
        // Encrypt data and save size

        data_len++; // We encrypt EOF too

        ciphertext = malloc((data_len + BLOCK_SIZE) * sizeof(uchar));
        if(ciphertext == NULL){
            printf("ERROR: Unable to allocate dynamic memory\n\n");
            exit(e_success);
        }
        uint cipher_len = 0;
        if (encrypt(data, (int) data_len, ciphertext, encInfo->enc, encInfo->enc_mode, encInfo->password, &cipher_len)){
            printf("INFO: Successfully encrypted the secret data\n");
        }else
        {
            printf("ERROR: Failed to encrypt secret data\n\n");
            return e_failure;
        }

        old_data = data;
        // Copying data for steganography
        data = malloc((INT_SIZE + cipher_len + CHAR_SIZE) * sizeof(uchar));
        if(data == NULL){
            printf("ERROR: Unable to allocate dynamic memory\n\n");
            exit(e_success);
        }
        // Copy size
        size_to_bytes(data, cipher_len);
        // Copy data
        memcpy(data + INT_SIZE, ciphertext, cipher_len);
        // Data length
        data_len = INT_SIZE + cipher_len;
        // EOF
        data[data_len] = '\0';
        printf("INFO: New secret data size %d\n", data_len);

        printf("INFO: Encrypted data ready for steganography\n");
    }

    /*
    //Encode secret data
    */
    printf("INFO: Encoding the secret data\n");
    if (encode_data(data, data_len, encInfo->fptr_src_image, encInfo->fptr_stego_image))
    {
        if(!lsbi_flag)
            printf("INFO: Successfully encoded the secret data\n");
    }
    else
    {
        printf("ERROR: Error while encoding the secret data\n\n");
        return e_failure;
    }

    // Set inverted flags and copy to stego file
    if(lsbi_flag){
        fseek(encInfo->fptr_src_image, SIZE_OF_HEADER, SEEK_SET);
        fseek(encInfo->fptr_stego_image, SIZE_OF_HEADER, SEEK_SET);
        if(set_inverted_flags(encInfo->fptr_src_image, encInfo->fptr_stego_image))
            printf("INFO: Successfully encoded inverted flags\n");
        else {
            printf("ERROR: Error while encoding inverted flags to stego file\n\n");
            return e_failure;
        }
        // Encode with inverted last bits
        // fseek(encInfo->fptr_src_image, SIZE_OF_HEADER + 4, SEEK_SET);
        // fseek(encInfo->fptr_stego_image, SIZE_OF_HEADER + 4, SEEK_SET);
        chosenStegoAlgorithm = lsbi_encode;
        if (encode_data(data, data_len, encInfo->fptr_src_image, encInfo->fptr_stego_image))
        {
            printf("INFO: Successfully encoded the secret data\n");
        }
        else
        {
            printf("ERROR: Error while encoding the secret data\n\n");
            return e_failure;
        }
    }

    // Copy rest of the file
    uchar c;
    uint aux = 0;
    while (fread(&c, sizeof(uchar), 1, encInfo->fptr_src_image))
    {
        fwrite(&c, sizeof(uchar), 1,encInfo->fptr_stego_image);
        aux++;
    }

    printf("INFO: Successfully copied all source file to stego file\n");

    // Clear memory
    free(data);
    if (password_flag){
        // Free old plain data and ciphertext
        free(old_data);
        free(ciphertext);
    }

    return e_success;//No error found
}

