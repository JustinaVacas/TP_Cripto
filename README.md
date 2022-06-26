# TP ESTEGANOGRAFÍA

## Requerimientos

- compilador C
- openSSL

## Compilacion

## Ejecucion

Los posibles argumentos son:

Los requeridos:
* extract: Indica que se va a extraer información.
* embed: Indica que se va a ocultar información.  
* p bitmapfile:	Archivo bmp portador  
* out file: Archivo de salida obtenido  
* steg <LSB1 | LSB4 | LSBI>: algoritmo de esteganografiado: LSB de 1bit, LSB de 4 bits, LSB Improved.  
* in file : Archivo que se va a ocultar.

Los opcionales: 
* pass password: password de encripcion 
* a: a <aes128 | aes192 | aes256 | des> 
* m <ecb|cfb|ofb|cbc>

### Ejemplos

```bash
$stegobmp -embed –in “mensaje1.txt” –p “imagen1.bmp” -out “imagenmas1.bmp” –steg LSBI –a des –m cbc -pass oculto
```

```bash
$stegobmp –extract –p “imagenmas1.bmp” -out “mensaje1” –steg LSBI –a des –m cbc -pass oculto
```

## Salida

Los archivos de salida en -embed se guardan en "stego_files" y los de -extract en "decoded". 
