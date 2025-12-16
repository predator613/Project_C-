#include "lz78.h"
#include <map>
#include <string>

const char* LZ78Compressor::getName() const { 
    return "LZ78"; 
}

CompressionResult LZ78Compressor::compress(const char* inputFile, const char* outputFile) {
    Timer timer;
    CompressionResult result;
    
    unsigned char* inputData = nullptr;
    long inputSize = 0;
    if (!readFile(inputFile, &inputData, &inputSize)) {
        return result;
    }
    
    timer.startTimer();
    
    unsigned char* outputData = new unsigned char[inputSize * 2];
    long outputPos = 0;
    
    std::map<std::string, int> dictionary;
    int nextCode = 1;
    std::string current;
    
    for (long i = 0; i < inputSize; i++) {
        std::string next = current + (char)inputData[i];
        if (dictionary.find(next) != dictionary.end()) {
            current = next;
        } else {
            int code = dictionary[current];
            outputData[outputPos++] = (code >> 8) & 0xFF;
            outputData[outputPos++] = code & 0xFF;
            outputData[outputPos++] = inputData[i];
            
            if (nextCode < 4096) {
                dictionary[next] = nextCode++;
            }
            current = "";
        }
    }
    
    if (!current.empty()) {
        int code = dictionary[current];
        outputData[outputPos++] = (code >> 8) & 0xFF;
        outputData[outputPos++] = code & 0xFF;
        outputData[outputPos++] = 0;
    }
    
    result.compressionTime = timer.stopTimer();
    result.originalSize = inputSize;
    result.compressedSize = outputPos;
    result.ratio = (double)outputPos / inputSize;
    result.success = writeFile(outputFile, outputData, outputPos);
    
    delete[] inputData;
    delete[] outputData;
    return result;
}

CompressionResult LZ78Compressor::decompress(const char* inputFile, const char* outputFile) {
    Timer timer;
    CompressionResult result;
    
    unsigned char* inputData = nullptr;
    long inputSize = 0;
    if (!readFile(inputFile, &inputData, &inputSize)) {
        return result;
    }
    
    timer.startTimer();
    
    unsigned char* outputData = new unsigned char[1000000];
    long outputPos = 0;
    long inputPos = 0;
    
    std::map<int, std::string> dictionary;
    int nextCode = 1;
    
    while (inputPos < inputSize - 2) {
        int code = (inputData[inputPos] << 8) | inputData[inputPos + 1];
        unsigned char ch = inputData[inputPos + 2];
        inputPos += 3;
        
        std::string phrase;
        if (code == 0) {
            phrase = std::string(1, ch);
        } else {
            phrase = dictionary[code] + std::string(1, ch);
        }
        
        for (char c : phrase) {
            outputData[outputPos++] = c;
        }
        
        if (nextCode < 4096) {
            dictionary[nextCode++] = phrase;
        }
    }
    
    result.decompressionTime = timer.stopTimer();
    result.success = writeFile(outputFile, outputData, outputPos);
    
    delete[] inputData;
    delete[] outputData;
    return result;
}