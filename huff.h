#include <stdio.h>
#include <stdlib.h>

typedef struct byte_t{
    unsigned short symb; /* Uncompressed symbol */
    unsigned long freq; /* Number of times char appears in file */
    unsigned long long prefix; /* Huffman prefix code assuming at  most 64 bit long*/
    short prefixLen; /* Length of prefix code */
    struct byte_t *lt, *rt; /* left and right pointes for Huffman tree */
} byte_t;

typedef struct heap_t{
    short size; /* Size of heap */
    byte_t *byte; /* Byte array */
} heap_t;

typedef struct tree_t{
    byte_t *root; /* Root node of tree */
} tree_t;

/* Discover file's filesize */
unsigned long file_len(FILE* file);

/* Initialize heap and set each byte's starting values */ void heap_init(heap_t *heap);

/* Free memory taken by heap */
void heap_dealloc(heap_t *heap);

/* Read file and determine character frequencies */
void heap_fill(heap_t *heap, FILE *file);

/* Eliminate characters that did not appear in file and determine heap size */
void heap_condense(heap_t *heap);

/* Used by heapify to sort by freq of symbol's occurence */
int heapCmp_freq(byte_t a, byte_t b);

/* Used by heapsort to sort by symbols order */
int heapCmp_symb(byte_t a, byte_t b);

/* Maintain heap property */
void heapify(heap_t *heap, int father, int (*cmp)(byte_t, byte_t));

/* Converts an unordered array into a heap*/
void heap_build(heap_t *heap, int (*cmp)(byte_t, byte_t));

/* Sort heap using heapsort with criteria defined by function cmp */
void heap_sort(heap_t *heap, int (*cmp)(byte_t, byte_t));

/* Duplicate a heap */
void heap_clone(heap_t original, heap_t* clone);

/* Extract smallest item from heap */
byte_t heap_extract_min(heap_t *heap, int(*cmp)(byte_t, byte_t));

/* Build huffman code tree */
byte_t* huff_build_tree(heap_t huffQueue);

/* Free memory taken by tree */
void tree_dealloc(byte_t *node);

/* Used by bsearch to find corect byte in a ordered array */
int bsearchCmp_symb(const void* a, const void* b);

/* Take a string with a binary code (little endian) and convert it to base 10 */
void bin_to_dec(char* binary, unsigned long long *dec, short bits);

/* Build the huffman prefix code for each character */
void huff_build_decode_table(heap_t *heap, tree_t tree);

/* Write prefix table header to file in format:
 * Number of symbols [2 bytes] Uncompressed file size [4 bytes]
 * For each symbol:
 *      Symbol [1 byte] Length of prefix code [1 byte] Prefix code [2 bytes]
 */
void file_write_header(FILE *outputFile, heap_t heap, unsigned long inputFileLen);

/* Convert a base 10 number into binary code (little endian) as a string*/
void dec_to_bin(unsigned long long dec, char* prefixBin, short bits);

/* Buffer used to write bitstream */
void write_bit(FILE *outputFile, unsigned long long prefix, short prefixLen);

/* Read input file and write compressed bitstream to output file */
void file_compress(FILE *outputFile, FILE *inputFile, heap_t heap, unsigned long inputFileLen);

/* Parse prefix table in file header and rebuild Huffman tree, return the original file's size */
unsigned long file_parse_header(FILE *compressedFile, tree_t *huff);

/* Decompress bitstream using encoding in the tree */
void file_decompress(FILE *compressedFile, FILE *outputFile, tree_t huff, unsigned long fileLen);
