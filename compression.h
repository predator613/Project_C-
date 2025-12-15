#ifndef COMPRESSION_H
#define COMPRESSION_H

#include <fstream>
#include <chrono>

class Timer {
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
public:
    void startTimer() { start = std::chrono::high_resolution_clock::now(); }
    double stopTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.0;
    }
};

struct CompressionResult {
    double compressionTime;
    double decompressionTime;
    long originalSize;
    long compressedSize;
    double ratio;
    bool success;
    
    CompressionResult() : compressionTime(0), decompressionTime(0), originalSize(0), 
                         compressedSize(0), ratio(0), success(false) {}
};

class Compressor {
public:
    virtual ~Compressor() = default;
    virtual CompressionResult compress(const char* inputFile, const char* outputFile) = 0;
    virtual CompressionResult decompress(const char* inputFile, const char* outputFile) = 0;
    virtual const char* getName() const = 0;
};

bool readFile(const char* filename, unsigned char** data, long* size);
bool writeFile(const char* filename, unsigned char* data, long size);

#endif