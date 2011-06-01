/* Joao Paulo Mendes de Sa
 * TP2 - Descompressor de Arquivos
 */

#include <stdio.h>
#include <stdlib.h>
#include "huff.h"
#define INFILE argv[2]
#define OUTFILE argv[1]

int main(int argc, char *argv[]){
    /* Open files */
    FILE *compressedFile = fopen(INFILE, "rb");
    FILE *outFile = fopen(OUTFILE, "wb");

    /* Rebuild huffman tree from header in compressed file */
    tree_t huff;
    unsigned long origFileLen = file_parse_header(compressedFile, &huff);

    /* Decompress the bit stream back to the original symbols using huffman code in tree*/
    file_decompress(compressedFile, outFile, huff, origFileLen);

    /* Close files */
    fclose(compressedFile);
    fclose(outFile);
    tree_dealloc(huff.root);
    return 0;
}
