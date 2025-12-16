#ifndef LZ78_H
#define LZ78_H

#include "../compression.h"

class LZ78Compressor : public Compressor {
public:
    const char* getName() const override;
    CompressionResult compress(const char* inputFile, const char* outputFile) override;
    CompressionResult decompress(const char* inputFile, const char* outputFile) override;
};

#endif