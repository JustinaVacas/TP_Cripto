#include "include/encode.h"
#include "include/decode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <getopt.h>

uint password_flag = 0;

int main(int argc, char* argv[]) {
    setbuf(stdout, NULL);
	EncodeInfo encInfo; //Structure variable
	//Putting default values
	encInfo.enc = AES_128;
	encInfo.enc_mode = CBC;
	uint encode_flag = 0;

	if (argc < 2) {
			printf("ERROR: Invalid number of command line arguments.\n\n");
			exit(e_success);
	}

	//
	/*	Parsing commands-line	*/
	//
	printf("\nINFO: Parsing command-line arguments..\n");
	int c;
    while (1){

        static struct option long_options[] = {
            {"embed",   no_argument,	0, 'e'},
            {"extract", no_argument,	0, 'd'},
            {"in",      required_argument, 0, 'i'},
            {"out",     required_argument, 0, 'o'},
            {"steg",    required_argument, 0, 's'},
            {"pass",    required_argument, 0, 'w'},
            {0,         0,                 0,  0}
		};

        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long_only (argc, argv, "edi:p:o:s:w:a:m:", long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;
        switch (c){
            case 'e': // Indica que se va a ocultar informacion.
			    encode_flag = 1;
				printf("INFO: Encoding method selected.\n");
                break;

            case 'd': // Indica que se va a extraer informacion.
				encode_flag = 0;
				printf("INFO: Decoding method selected.\n");
                break;

            case 'i': // parametro: Archivo que se va a ocultar.
				printf("INFO: Verifying secret filename...\n");
				if (encode_flag)
				{
					if (read_and_validate_extn((uchar_ptr)basename(optarg), &encInfo))
					{
						printf("INFO: Valid secret filename: %s\n", optarg);
					}
					else
					{
						printf("ERROR: Invalid secret filename.\n\n");
						exit(e_success);
					}
					strcpy((char*)encInfo.secret_fname, optarg);
				}
				else{
					printf("ERROR: Invalid command '-in' for decoding method.\n\n");
					exit(e_success);
				}
                break;

            case 'p': // parametro: Archivo bmp portador
				if (encode_flag)
				{
					//Extract only filename from given path if any
					//optarg = basename(optarg);
					printf("INFO: Verifying source image filename..\n");
					if (read_and_validate_bmp_format(basename(optarg)) == e_failure) {
						printf("ERROR: Invalid source image. It must be a '.bmp' file\n\n");
						exit(e_success);
					}
					printf("INFO: Valid source image filename: %s\n", optarg);
					strcpy((char*)encInfo.src_image_fname, optarg);
				}
				else
				{
					printf("INFO: Verifying stego image filename..\n");
					if (read_and_validate_bmp_format(basename(optarg)) == e_failure) {
						printf("ERROR: Invalid stego image. It must be a '.bmp' file\n\n");
						exit(e_success);
					}
					printf("INFO: Valid stego image filename: %s\n", optarg);
                    strcpy((char*)encInfo.stego_image_fname, optarg);
				}
                break;

            case 'o': // parametro: Archivo bmp de salida, es decir, el archivo bitmapfile con la informacion de file incrustada. O archivo de salida obtenido.
                optarg = basename(optarg);
                if (encode_flag)
				{
					//Extract only filename from given path if any
					printf("INFO: Verifying stego image filename..\n");
					if (read_and_validate_bmp_format(optarg) == e_failure) {
						printf("ERROR: Invalid stego image. It must be a '.bmp' file\n\n");
						exit(e_success);
					}
					printf("INFO: Valid stego image filename: %s\n", optarg);
                    strcpy((char*)encInfo.stego_image_fname, "./stego_files/");
					strcat((char*)encInfo.stego_image_fname, optarg);
				}else
				{
					printf("INFO: Valid secret filename: %s\n", optarg);
                    strcpy((char*)encInfo.secret_fname, "./decoded/");
					strcat((char*)encInfo.secret_fname, optarg);
				}
                break;

            case 's': // parametro: algoritmo de esteganografiado: <LSB1 | LSB4 | LSBI>
                if (strcmp(optarg, "LSB1") == 0)
				{
					encInfo.stego_mode = LSB1;
				}else if (strcmp(optarg, "LSB4") == 0)
				{
					encInfo.stego_mode = LSB4;
				}else if (strcmp(optarg, "LSBI") == 0)
				{
					encInfo.stego_mode = LSBI;
				}else
				{
					printf("ERROR: Invalid stego mode argument %s. Use <LSB1 | LSB4 | LSBI>\n\n", optarg);
					exit(e_success);
				}
				printf("INFO: %s stego type selected\n", optarg);
                break;
            
            case 'w': // parametro: password (password de encripcion)
				if (strlen(optarg) < MAX_PASSWORD_LEN)
				{
					printf("INFO: Password = \"%s\"\n", optarg);
					strcpy((char*)encInfo.password, optarg);
					password_flag = 1;
				}
				else
				{
					printf("ERROR: Password %s too long. Maximum password lenght is %d\n\n", optarg, MAX_PASSWORD_LEN);
					exit(e_success);
				}
                break;

            case 'a': // parametro: <aes128 | aes192 | aes256 | des>
                if (strcmp(optarg, "aes128") == 0)
				{
					encInfo.enc = AES_128;
				}else if (strcmp(optarg, "aes192") == 0)
				{
					encInfo.enc = AES_192;
				}else if (strcmp(optarg, "aes256") == 0)
				{
					encInfo.enc = AES_256;
				}else if (strcmp(optarg, "des") == 0)
				{
					encInfo.enc = DES;
				}else
				{
					printf("ERROR: Invalid encryption argument %s. Use <aes128 | aes192 | aes256 | des>\n\n", optarg);
					exit(e_success);
				}
				printf("INFO: Encription selected: %s\n", optarg);
                break;

            case 'm': // parametro modo de encripción: <ecb | cfb | ofb | cbc>
                if (strcmp(optarg, "ecb") == 0)
				{
					encInfo.enc_mode = ECB;
				}else if (strcmp(optarg, "cfb") == 0)
				{
					encInfo.enc_mode = CFB;
				}else if (strcmp(optarg, "ofb") == 0)
				{
					encInfo.enc_mode = OFB;
				}else if (strcmp(optarg, "cbc") == 0)
				{
					encInfo.enc_mode = CBC;
				}else
				{
					printf("ERROR: Invalid encryption mode argument %s. Use <ecb | cfb | ofb | cbc>\n\n", optarg);
					exit(e_success);
				}
				printf("INFO: Encription mode selected: %s\n", optarg);
                break;
            default:
                fprintf(stderr, "Invalid argument %d\n", c);
                exit(1);
        }
    }

    if (optind < argc)
    {
      printf ("Arguments not accepted: ");
      while (optind < argc)
        printf ("%s ", argv[optind++]);
      putchar ('\n');
    }
	printf("INFO: End of arguments parsing\n\n");
	//
	// End of parsing operation...
	//

	//
	//	Enconding opertation	
	//
	if (encode_flag){
		printf("INFO: Encoding operation requested\n");

		// Test open_files
		printf("INFO: Opening all the necessary files...\n");
		if (open_files(&encInfo) == e_success) {
				printf("INFO: All files successfully opened\n\n");
		} 
		else {
				exit(e_success);
		}

		printf("INFO: Obtaining image size\n");
		encInfo.src_image_size = get_image_size_for_bmp(encInfo.fptr_src_image);
		printf("INFO: Image size = %u bytes\n", encInfo.src_image_size);

		printf("INFO: Checking compression type is 0\n");
		if(!check_compression(encInfo.fptr_src_image)){
			printf("ERROR: Compression type on source bmp file must be 0\n\n");
			exit(e_success);
		}
		printf("INFO: Compression OK\n");

		if (copy_bmp_header((FILE*) encInfo.fptr_src_image, (FILE*) encInfo.fptr_stego_image)) {
			printf("INFO: Image header copied to output file successfully\n");
		} else {
			printf("ERROR: Failed to copy image header\n\n");
			exit(e_success);
		}

		printf("INFO: Verifying secret file size...\n");
		encInfo.secret_fsize = get_file_size(encInfo.fptr_secret);
		if (!encInfo.secret_fsize)
		{
				printf("ERROR: Secret file to be encoded is empty\n\n");
				exit(e_success);
		}
		printf("INFO: Secret file is not empty\n");
		printf("INFO: Secret data size = %d bytes\n", encInfo.secret_fsize);//Last byte i.e. EOF is not considered in actual secret data size

		//Store image size in bytes and check capacity
		printf("INFO: Verifying encoding capacity...\n");
		if(!check_capacity(&encInfo)){
			exit(e_success);
		}
		printf("INFO: Capacity OK\n");

		printf("\nINFO: ##--------Encoding procedure started---------##\n");
		if (do_encoding(&encInfo))
		{
				printf("INFO: ##------Encoding operation successful!!------##\n\n");
		}
		else
		{
				printf("ERROR: ##------Encoding operation failed!!------##\n\n");
				exit(e_success);
		}

		fclose(encInfo.fptr_src_image);//close source image file
		fclose(encInfo.fptr_secret);//Close secret file
	}

	//
	//	Decoding opertation	
	//
	else{
        printf("INFO: Decoding operation requested\n");

        //Open stegged image file
        printf("INFO: Opening the image file\n");
        if((encInfo.fptr_stego_image = fopen((const char*)encInfo.stego_image_fname, "rb")) == NULL)
        {
                printf("ERROR: Unable to open file %s. This file may not be present in the current project directory\n\n", encInfo.stego_image_fname);
                exit(e_success);
        }
        printf("INFO: Image file successfully opened\n\n");

        printf("INFO: Obtaining image size\n");
        encInfo.stego_fsize = get_image_size_for_bmp(encInfo.fptr_stego_image);
        printf("INFO: Image size = %u bytes\n", encInfo.stego_fsize);

        fseek(encInfo.fptr_stego_image, SIZE_OF_HEADER, SEEK_SET);
        if (ferror(encInfo.fptr_stego_image)) {
                printf("ERROR: Error while reading file %s\n\n", encInfo.stego_image_fname);
                exit(e_success);
        }
        //Stego file index is now pointing at the end of raster data

        //Decode File
        printf("INFO: ##--------Decoding procedure started--------##\n");
        if (do_decoding(&encInfo))
        {
                printf("INFO: ##------Decoding operation successful!!------##\n\n");
        }
        else
        {
                printf("ERROR: ##------Decoding operation failed!!------##\n\n");
                exit(e_success);
        }

        //close decoded output file
        fclose(encInfo.fptr_secret);
	}

	//Close the output file
	fclose(encInfo.fptr_stego_image);//common file both in encoding & decoding part
	return 0;
}
