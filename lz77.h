#ifndef LZ77_H
#define LZ77_H

#include "../compression.h"

class LZ77Compressor : public Compressor {
public:
    const char* getName() const override;
    CompressionResult compress(const char* inputFile, const char* outputFile) override;
    CompressionResult decompress(const char* inputFile, const char* outputFile) override;
};

#endif