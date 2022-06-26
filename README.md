# TP ESTEGANOGRAFÍA
##

### Requerimientos

- Compilador gcc
- Libreria openSSL (libssl-dev)

#### Compilacion

Para crear el ejecutable stegobmp ejecutar
```bash
make all
```

#### Limpieza

Para limpiar la carpeta ejecutar
```bash
make clean
```
##
## Ocultamiento de un archivo en un .bmp

El programa debe recibir como parámetros:

* -embed
Indica que se va a ocultar información.
* -in <file>
Archivo que se va a ocultar.
* -p <bitmapfile>
Archivo bmp que será el portador.
* -out <bitmapfile>
Archivo bmp de salida.
* -steg <LSB1 | LSB4 | LSBI>
Algoritmo de esteganografiado: LSB de 1bit, LSB de 4 bits, LSB Improved.

Y los siguientes parámetros opcionales de encriptado:
* -a <aes128 (default)| aes192 | aes256 | des>
* -m <ecb | cfb | ofb | cbc(default)>
* -pass <password>

#### Ejemplos

Con encriptado
```bash
$stegobmp -embed –in mensaje1.txt –p imagen1.bmp -out imagenmas1.bmp –steg LSBI –a des –m cbc -pass oculto
```

Sin encriptado
```bash
$stegobmp -embed –in mensaje1.txt –p imagen1.bmp -out imagenmas1.bmp –steg LSBI
```
##
## Extraer de un archivo .bmp un archivo oculto

El programa debe recibir como parámetros:

* -extract
Indica que se va a extraer información.
* -p <bitmapfile>
Archivo bmp portador.
* -out <file>
Archivo de salida obtenido.
* -steg <LSB1 | LSB4 | LSBI>
Algoritmo de esteganografiado: LSB de 1bit, LSB de 4 bits, LSB Improved.

Y los siguientes parámetros opcionales de desencriptado:
* -a <aes128 | aes192 | aes256 | des>
* -m <ecb | cfb | ofb | cbc>
* -pass <password> (password de encripcion)


#### Ejemplos

Con encriptado
```bash
$stegobmp –extract –p imagenmas1.bmp -out mensaje1 –steg LSBI –a des –m cbc -pass oculto
```

Sin encriptado
```bash
$stegobmp –extract –p imagenmas1.bmp -out mensaje1 –steg LSBI
```
##
## Salida

Los archivos de salida de -embed se guardan en la carpeta "stego_files" y los archivos de salida de -extract en la carpeta "decoded".
