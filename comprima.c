/* Joao Paulo Mendes de Sa
 * TP2 - Compressor de Arquivos
 */

#include <stdio.h>
#include <stdlib.h>
#include "huff.h"
#define INFILE argv[2]
#define OUTFILE argv[1]

int main(int argc, char *argv[]){
    /* Open files */
    FILE *infile = fopen(INFILE, "rb");
    unsigned long infileLen = file_len(infile);
    FILE *outfile = fopen(OUTFILE, "wb");

    heap_t huffQueue;
    tree_t huffTree;

    /* First pass on the input file to gather symbols and symbol frequencies */
    heap_init(&huffQueue);
    heap_fill(&huffQueue, infile);
    heap_condense(&huffQueue);

    /* Order priority queue by frequency and begin building a Huffman tree */
    heap_build(&huffQueue, heapCmp_freq);
    huffTree.root = huff_build_tree(huffQueue);

    /* Reorder the heap alphabetically and navigate the Huffman tree to determine prefix codes */
    heap_sort(&huffQueue, heapCmp_symb);
    huff_build_decode_table(&huffQueue, huffTree);

    /* Second pass to actually compress input file into output file's bitstream, output file's bitstream preceded by header containing necessary information for decoding */
    file_write_header(outfile, huffQueue, infileLen);
    file_compress(outfile, infile, huffQueue, infileLen);

    /* When done close files */
    fclose(infile);
    fclose(outfile);
    heap_dealloc(&huffQueue);
    tree_dealloc(huffTree.root);
    return 0;
}
