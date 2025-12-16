#include "lz77.h"
#include <cstring>
#include <fstream>
#include <iostream>

using namespace std;

#pragma pack(push, 1)
struct LZ77Token {
    unsigned short offset;
    unsigned char length;
    char nextChar;
    LZ77Token(unsigned short o = 0, unsigned char l = 0, char c = '\0')
        : offset(o), length(l), nextChar(c) {
    }
};
#pragma pack(pop)

const char* LZ77Compressor::getName() const { 
    return "LZ77"; 
}

CompressionResult LZ77Compressor::compress(const char* inputFile, const char* outputFile) {
    Timer timer;
    CompressionResult result;
    
    unsigned char* inputData;
    long inputSize;
    if (!readFile(inputFile, &inputData, &inputSize)) return result;
    
    timer.startTimer();

    string inputString(reinterpret_cast<char*>(inputData), inputSize);
    LZ77Token* compressed = nullptr;
    int compressedSize = 0;
    
    int windowSize = 4096;
    int lookaheadSize = 18;
    int minMatchLength = 3;
    
    int inputLength = inputString.length();
    int capacity = inputLength;
    compressed = new LZ77Token[capacity];
    compressedSize = 0;
    int pos = 0;

    while (pos < inputLength) {
        int matchOffset = 0;
        int matchLength = 0;
        int searchStart = (pos - windowSize > 0) ? (pos - windowSize) : 0;
        int maxLength = (pos + lookaheadSize < inputLength) ? lookaheadSize : (inputLength - pos);

        for (int i = searchStart; i < pos; i++) {
            int currentLength = 0;
            while (currentLength < maxLength &&
                i + currentLength < pos &&
                pos + currentLength < inputLength &&
                inputString[i + currentLength] == inputString[pos + currentLength]) {
                currentLength++;
            }
            if (currentLength > matchLength) {
                matchLength = currentLength;
                matchOffset = pos - i;
            }
        }

        if (matchLength < minMatchLength) {
            matchOffset = 0;
            matchLength = 0;
        }

        char nextChar = (pos + matchLength < inputLength) ? inputString[pos + matchLength] : '\0';
        compressed[compressedSize++] = LZ77Token(matchOffset, matchLength, nextChar);
        pos += (matchLength + 1);
    }
    
    ofstream file(outputFile, ios::binary);
    if (!file) {
        delete[] inputData;
        delete[] compressed;
        result.success = false;
        return result;
    }
    
    file.write(reinterpret_cast<const char*>(&compressedSize), sizeof(compressedSize));
    for (int i = 0; i < compressedSize; i++) {
        file.write(reinterpret_cast<const char*>(&compressed[i]), sizeof(LZ77Token));
    }
    file.close();
    
    result.compressionTime = timer.stopTimer();
    result.originalSize = inputSize;
    result.compressedSize = sizeof(compressedSize) + compressedSize * sizeof(LZ77Token);
    result.ratio = (double)result.compressedSize / inputSize;
    result.success = true;
    
    delete[] inputData;
    delete[] compressed;
    return result;
}

CompressionResult LZ77Compressor::decompress(const char* inputFile, const char* outputFile) {
    Timer timer;
    CompressionResult result;
    
    ifstream file(inputFile, ios::binary);
    if (!file) {
        result.success = false;
        return result;
    }
    
    timer.startTimer();
    
    int compressedSize;
    file.read(reinterpret_cast<char*>(&compressedSize), sizeof(compressedSize));
    LZ77Token* compressed = new LZ77Token[compressedSize];
    file.read(reinterpret_cast<char*>(compressed), compressedSize * sizeof(LZ77Token));
    file.close();
    
    string decompressed;
    for (int i = 0; i < compressedSize; i++) {
        const auto& token = compressed[i];
        if (token.length > 0) {
            int startPos = decompressed.length() - token.offset;
            for (int j = 0; j < token.length; j++) {
                decompressed += decompressed[startPos + j];
            }
        }
        if (token.nextChar != '\0') {
            decompressed += token.nextChar;
        }
    }
    
    ofstream outFile(outputFile, ios::binary);
    if (!outFile) {
        delete[] compressed;
        result.success = false;
        return result;
    }
    outFile.write(decompressed.c_str(), decompressed.length());
    outFile.close();
    
    result.decompressionTime = timer.stopTimer();
    result.success = true;
    
    delete[] compressed;
    return result;
}