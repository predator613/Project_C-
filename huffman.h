#ifndef HUFFMAN_H
#define HUFFMAN_H

#include "../compression.h"

class HuffmanCompressor : public Compressor {
public:
    const char* getName() const override;
    CompressionResult compress(const char* inputFile, const char* outputFile) override;
    CompressionResult decompress(const char* inputFile, const char* outputFile) override;
};

#endif