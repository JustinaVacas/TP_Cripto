#include "encode.h"
#include "header.h"

#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

int main (int argc, char **argv){

  static struct args args;

    int c;
    while (1){
        static struct option long_options[] = {
          {"embed",   no_argument,       0, 'e'},
          {"extract", required_argument, 0, 'x'},
          {"in",      required_argument, 0, 'i'},
          {"p",       required_argument, 0, 'p'},
          {"out",     required_argument, 0, 'o'},
          {"steg",    required_argument, 0, 's'},
          {"pass",    required_argument, 0, 'w'},
          {"a",       optional_argument, 0, 'a'},
          {"m",       optional_argument, 0, 'm'},
          {0,         0,                 0,  0}};

        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long (argc, argv, "exi:p:o:s:w:a:m", long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c){
            case 'e': // Indica que se va a ocultar informacion.
            
                break;

            case 'x': // Indica que se va a extraer informacion.

                break;

            case 'i': // parametro: Archivo que se va a ocultar.
                args->in = optarg
                break;

            case 'p': // parametro: Archivo bmp portador
                args->p = optarg
                break;

            case 'o': // parametro: Archivo bmp de salida, es decir, el archivo bitmapfile con la informacion de file incrustada. O archivo de salida obtenido.
                args->out = optarg
                break;

            case 's': // parametro: algoritmo de esteganografiado: LSB de 1bit, LSB de 4 bits, LSB Enhanced
                args->steg = optarg
                break;
            
            case 'w': // parametro: password (password de encripcion)
                args->pass = optarg
                break;

            case 'a': // parametro: <aes128 | aes192 | aes256 | des>
                args->enc = get_enc(optarg)
                break;

            case 'm': // parametro: <ecb|cfb|ofb|cbc>
                args->mode = optarg
                break;

            default:
                fprintf(stderr, "Argumento invalido %d.\n", c);
                exit(1);
        }
    }

    if (optind < argc)
    {
      printf ("Argumentos no aceptados: ");
      while (optind < argc)
        printf ("%s ", argv[optind++]);
      putchar ('\n');
    }


    BITMAPINFOHEADER bitmapInfoHeader;
    BITMAPFILEHEADER bitmapFileHeader;
    unsigned char *bitmapData;

    bitmapData = LoadBitmapFile("./examples/bogota.bmp",&bitmapInfoHeader,&bitmapFileHeader);
    
    exit (0);
}

