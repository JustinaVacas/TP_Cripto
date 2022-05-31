#include "encode.h"
#include "header.h"

int main(){
    BITMAPINFOHEADER bitmapInfoHeader;
    BITMAPFILEHEADER bitmapFileHeader;
    unsigned char *bitmapData;

    bitmapData = LoadBitmapFile("./examples/bogota.bmp",&bitmapInfoHeader,&bitmapFileHeader);
    return 0;
}