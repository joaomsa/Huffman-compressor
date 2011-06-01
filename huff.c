#include <stdio.h>
#include <stdlib.h>
#include "huff.h"

/* Discover file's filesize */
unsigned long file_len(FILE* file){
    unsigned long len;
    fseek(file, 0, SEEK_END);
    len = ftell(file);
    fseek(file, 0, SEEK_SET);
    return len;
}

/* Initialize heap and set each byte's starting values */
void heap_init(heap_t *heap){
    short i;
    heap->byte = (byte_t*) malloc(256 * sizeof(byte_t));
    heap->size = 0;
    for (i = 0; i < 256; i++){
        heap->byte[i].symb = i;
        heap->byte[i].freq = 0;
        heap->byte[i].lt = NULL;
        heap->byte[i].rt = NULL;
    }
}

/* Free memory taken by heap */
void heap_dealloc(heap_t *heap){
    free(heap->byte);
}

/* Read file and determine character frequencies */
void heap_fill(heap_t *heap, FILE *file){
    unsigned long size = file_len(file);
    unsigned long i;
    unsigned char *fileBuffer = (unsigned char*) malloc(size * sizeof(unsigned char));
    fread(fileBuffer, sizeof(unsigned char), size, file);
    for (i = 0; i < size; i++){
        heap->byte[fileBuffer[i]].freq++;
    }
    free(fileBuffer);
}

/* Eliminate characters that did not appear in file and determine heap size */
void heap_condense(heap_t *heap){
    int i, j;
    for (i = 0; i < 256; i++){
        if (heap->byte[i].freq == 0){
            for (j = i + 1; j < 256 && heap->byte[j].freq == 0; j++);
            if (j < 256){
                heap->byte[i] = heap->byte[j];
                heap->byte[j].freq = 0;
                heap->size++;
            }
        }
        else
            heap->size++;
    }
}

/* Used by heapify to sort by freq of symbol's occurence */
int heapCmp_freq(byte_t a, byte_t b){
    if (a.freq < b.freq)
        return 1;
    else
        return 0;
}

/* Used by heapsort to sort by symbols order */
int heapCmp_symb(byte_t a, byte_t b){
    if (a.symb > b.symb)
        return 1;
    else
        return 0;
}

/* Maintain heap property */
void heapify(heap_t *heap, int father, int (*cmp)(byte_t, byte_t)){
    int left = father * 2 + 1;
    int right = father * 2 + 2;
    int smallest;
    byte_t aux;
    if (left < heap->size && cmp(heap->byte[left], heap->byte[father]))
        smallest = left;
    else
        smallest = father;
    if (right < heap->size && cmp(heap->byte[right], heap->byte[smallest]))
        smallest = right;
    if (smallest != father){
        aux = heap->byte[father];
        heap->byte[father] = heap->byte[smallest];
        heap->byte[smallest] = aux;
        heapify(heap, smallest, cmp);
    }
}

/* Converts an unordered array into a heap*/
void heap_build(heap_t *heap, int (*cmp)(byte_t, byte_t)){
    int i;
    for (i = (heap->size - 1) / 2; i >= 0; i--)
        heapify(heap, i, cmp);
}

/* Sort heap using heapsort with criteria defined by function cmp */
void heap_sort(heap_t *heap, int (*cmp)(byte_t, byte_t)){
    int i;
    int heapLen = heap->size;
    byte_t aux;
    heap_build(heap, cmp);
    for (i = heap->size - 1; i > 0; i--){
        aux = heap->byte[0];
        heap->byte[0] = heap->byte[i];
        heap->byte[i] = aux;
        heap->size--;
        heapify(heap, 0, cmp);
    }
    heap->size = heapLen;
}

/* Duplicate a heap */
void heap_clone(heap_t original, heap_t* clone){
    int i;
    for (i = 0; i < original.size; i++)
        clone->byte[i] = original.byte[i];
    clone->size = original.size;
}

/* Extract smallest item from heap */
byte_t heap_extract_min(heap_t *heap, int(*cmp)(byte_t, byte_t)){
    byte_t smallest;
    smallest = heap->byte[0];
    heap->byte[0] = heap->byte[heap->size - 1];
    heap->size--;
    heapify(heap, 0, cmp);
    return smallest;
}

/* Build huffman code tree */
byte_t* huff_build_tree(heap_t huffQueue){
    heap_t huffClone;
    int i;
    byte_t *auxLt, *auxRt;
    byte_t *auxFa;
    heap_t huffAux;

    /* Instead of reordering the heap everytime we reinsert a node just use an auxilary heap and always insert at the end */
    heap_init(&huffClone);
    heap_clone(huffQueue, &huffClone);
    heap_init(&huffAux);

    /* Make sure that the unitialized value is the biggest possible */
    for (i = 0; i < huffQueue.size; i++)
        huffAux.byte[i].freq = 0xffffffff;

    for (i = 0; i < huffQueue.size - 1; i++){
        auxLt = (byte_t*) malloc(sizeof(byte_t));
        auxRt = (byte_t*) malloc(sizeof(byte_t));
        if (huffClone.size > 0 && huffClone.byte[0].freq <= huffAux.byte[0].freq)
            *auxLt = heap_extract_min(&huffClone, heapCmp_freq);
        else
            *auxLt = heap_extract_min(&huffAux, heapCmp_freq);
        if (huffClone.size > 0 && (huffAux.size == 0 || huffClone.byte[0].freq <= huffAux.byte[0].freq))
            *auxRt = heap_extract_min(&huffClone, heapCmp_freq);
        else
            *auxRt = heap_extract_min(&huffAux, heapCmp_freq);

        huffAux.byte[huffAux.size].freq = auxLt->freq + auxRt->freq;
        huffAux.byte[huffAux.size].lt = auxLt;
        huffAux.byte[huffAux.size].rt = auxRt;
        huffAux.byte[huffAux.size].symb = 'F';
        huffAux.size++;
    }
    auxFa = (byte_t*) malloc(sizeof(byte_t));
    *auxFa = huffAux.byte[0];
    heap_dealloc(&huffClone);
    heap_dealloc(&huffAux);
    return auxFa;
}

/* Free memory taken by tree */
void tree_dealloc(byte_t *node){
    if (node->lt != NULL)
        tree_dealloc(node->lt);
    if (node->rt != NULL)
        tree_dealloc(node->rt);
    free(node);
}

/* Used by bsearch to find corect byte in a ordered array */
int bsearchCmp_symb(const void* a, const void* b){
    byte_t *byte1 = (byte_t*) a;
    byte_t *byte2 = (byte_t*) b;
    return byte1->symb - byte2->symb;
}

/* Take a string with a binary code (little endian) and convert it to base 10 */
void bin_to_dec(char* binary, unsigned long long *dec, short bits){
    int i;
    *dec = 0;
    for (i = 0; i < bits; i++)
        if (binary[i] == 1)
            *dec += 1 << i;
}

/* Build the huffman prefix code for each character */
void huff_build_decode_table(heap_t *heap, tree_t tree){
    void huff_build_decode_table_aux(heap_t *heap, byte_t byte, char *prefixBin, short prefixLen){
        int i;
        byte_t *matchedSymb;
        if (byte.lt == NULL && byte.rt == NULL){
            matchedSymb = bsearch(&byte, heap->byte, heap->size, sizeof(byte_t), bsearchCmp_symb);
            if (matchedSymb != NULL){
                for (i = prefixLen; i < 64; i++)
                    prefixBin[i] = 0;
                matchedSymb->prefixLen = prefixLen;
                bin_to_dec(prefixBin, &matchedSymb->prefix, 64);
            }
        }
        prefixBin[prefixLen] = 0;
        if (byte.lt != NULL)
            huff_build_decode_table_aux(heap, *byte.lt, prefixBin, prefixLen + 1);
        prefixBin[prefixLen] = 1;
        if (byte.rt != NULL)
            huff_build_decode_table_aux(heap, *byte.rt, prefixBin, prefixLen + 1);
    }
    char prefixBin[64];
    huff_build_decode_table_aux(heap, *tree.root, prefixBin, 0);
}

/* Write prefix table header to file in format:
 * Number of symbols [2 bytes] Uncompressed file size [4 bytes]
 * For each symbol:
 *      Symbol [1 byte] Length of prefix code [1 byte] Prefix code [8 bytes]
 */
void file_write_header(FILE *outputFile, heap_t heap, unsigned long inputFileLen){
    int i;
    unsigned char symb, prefixLen;
    fwrite(&heap.size, sizeof(short), 1, outputFile);
    fwrite(&inputFileLen, sizeof(unsigned long), 1, outputFile);
    for (i = 0; i < heap.size; i++){
        symb = heap.byte[i].symb;
        prefixLen = heap.byte[i].prefixLen;
        fwrite(&symb, sizeof(unsigned char), 1, outputFile);
        fwrite(&prefixLen, sizeof(unsigned char), 1, outputFile);
        fwrite((unsigned long long*)&heap.byte[i].prefix, sizeof(unsigned long long), 1, outputFile);
    }
}

/* Convert a base 10 number into binary code (little endian) as a string*/
void dec_to_bin(unsigned long long dec, char* prefixBin, short bits){
    int i;
    unsigned long long shift = 1;
    for (i = bits - 1; i >= 0; i--){
        if (dec / (shift << i))
            prefixBin[i] = 1;
        else
            prefixBin[i] = 0;
        dec = dec % (shift << i);
    }
}

/* Buffer used to write bitstream */
void write_bit(FILE *outputFile, unsigned long long prefix, short prefixLen){
    static unsigned char buffer = 0; /* Persistent across function calls */
    static unsigned bufferUsed = 0;
    int i;
    for (i = 0; i < prefixLen; i++){
        buffer <<= 1; /* Shift one position to make room in buffer */
        buffer |= ((prefix & 1) != 0); /* Set the last bit from buffer to the first bit of the prefix code */
        prefix >>= 1; /* Discard bit last set in buffer */

        bufferUsed++;
        if (bufferUsed == 8){ /* Once buffer filled with 8 bits write the byte */
            fwrite(&buffer, sizeof(unsigned char), 1, outputFile);
            buffer = 0; /* Clear buffer */
            bufferUsed = 0;
        }
    }
}

/* Read input file and write compressed bitstream to output file */
void file_compress(FILE *outputFile, FILE *inputFile, heap_t heap, unsigned long inputFileLen){
    unsigned long i;
    byte_t *matchedSymb;
    byte_t symbRead;


    fseek(inputFile, 0, SEEK_SET);
    for (i = 0; i < inputFileLen; i++){
        symbRead.symb = fgetc(inputFile);
        matchedSymb = bsearch(&symbRead, heap.byte, heap.size, sizeof(byte_t), bsearchCmp_symb);
        write_bit(outputFile, matchedSymb->prefix, matchedSymb->prefixLen);
    }
    write_bit(outputFile, 255,7); /* Write last partial bit in buffer to file */
}

/* Parse prefix table in file header and rebuild Huffman tree, return the original file's size */
unsigned long file_parse_header(FILE *compressedFile, tree_t *huff){
    unsigned char symb, prefixLen;
    unsigned long long prefixDec;
    short symbNum;
    char prefixBin[64]; 
    unsigned long origFileLen;
    int i, j;
    byte_t *aux;

    /* Do nothing if empty file */
    if (file_len(compressedFile) == 0)
        exit(0);

    fread(&symbNum, sizeof(short), 1, compressedFile);
    fread(&origFileLen, sizeof(unsigned long), 1, compressedFile);
    huff->root = (byte_t*) malloc(sizeof(byte_t));
    huff->root->lt = NULL;
    huff->root->rt = NULL;
    aux = huff->root;

    for (i = 0; i < symbNum; i++){
        fread(&symb, sizeof(unsigned char), 1, compressedFile);
        fread(&prefixLen, sizeof(unsigned char), 1, compressedFile);
        fread(&prefixDec, sizeof(unsigned long long), 1, compressedFile);
        dec_to_bin(prefixDec, prefixBin, 64);

        for (j = 0; j < prefixLen; j++){
            if (prefixBin[j] == 0){
                if (aux->lt == NULL){
                    aux->lt = (byte_t*) malloc(sizeof(byte_t));
                    aux->lt->lt = NULL;
                    aux->lt->rt = NULL;
                }
                aux = aux->lt;
            }
            else {
                if (aux->rt == NULL){
                    aux->rt = (byte_t*) malloc(sizeof(byte_t));
                    aux->rt->lt = NULL;
                    aux->rt->rt = NULL;
                }
                aux = aux->rt;
            }
        }
        aux->symb = symb;
        aux->prefixLen = prefixLen;
        aux->prefix = prefixDec;
        aux->lt = NULL;
        aux->rt = NULL;
        aux = huff->root;
    }
    return origFileLen;
}

/* Decompress bitstream using encoding in the tree */
void file_decompress(FILE *compressedFile, FILE *outputFile, tree_t huff, unsigned long fileLen){
    int bufferUsed= 0;
    char readBuffer[8];
    unsigned char readBufferDec;
    byte_t *aux = huff.root;
    unsigned long i = 0;
    while (i < fileLen){
        if (aux->lt == NULL && aux->rt == NULL){
            fputc(aux->symb, outputFile);
            aux = huff.root;
            i++;
        }
        else if (bufferUsed > 0){
            if (readBuffer[bufferUsed - 1] == 0)
                aux = aux->lt;
            else
                aux = aux->rt;
            bufferUsed--;
        }
        else {
            fread(&readBufferDec, 1, 1, compressedFile);
            dec_to_bin(readBufferDec, readBuffer, 8);
            bufferUsed = 8;
        }
    }
}
