#ifndef COMPRESSORS_H
#define COMPRESSORS_H

#include "compression.h"

Compressor* createHuffmanCompressor();
Compressor* createLZSSCompressor();
Compressor* createLZ77Compressor();
Compressor* createLZ78Compressor();
Compressor* createLZWCompressor();

#endif