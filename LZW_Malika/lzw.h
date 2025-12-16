#ifndef LZW_H
#define LZW_H

#include "../compression.h"

class LZWCompressor : public Compressor {  // ДОЛЖНО БЫТЬ LZWCompressor, а не LZMCompressor
public:
    const char* getName() const override;
    CompressionResult compress(const char* inputFile, const char* outputFile) override;
    CompressionResult decompress(const char* inputFile, const char* outputFile) override;
};

#endif