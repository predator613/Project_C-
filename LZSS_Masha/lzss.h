#ifndef LZSS_H
#define LZSS_H

#include "../compression.h"

class LZSSCompressor : public Compressor {
public:
    const char* getName() const override;
    CompressionResult compress(const char* inputFile, const char* outputFile) override;
    CompressionResult decompress(const char* inputFile, const char* outputFile) override;
};

#endif