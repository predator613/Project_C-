#include "lzw.h"
#include <map>
#include <string>
#include <iostream>

const char* LZWCompressor::getName() const { 
    return "LZW"; 
}

CompressionResult LZWCompressor::compress(const char* inputFile, const char* outputFile) {
    Timer timer;
    CompressionResult result;
    result.success = false;
    
    unsigned char* inputData = nullptr;
    long inputSize = 0;
    if (!readFile(inputFile, &inputData, &inputSize)) {
        return result;
    }
    
    if (inputSize <= 0) {
        delete[] inputData;
        return result;
    }
    
    timer.startTimer();
    
    try {
        std::map<std::string, int> dictionary;
        for (int i = 0; i < 256; i++) {
            dictionary[std::string(1, static_cast<char>(i))] = i;
        }
        
        int dictSize = 256;
        unsigned char* outputData = new unsigned char[inputSize * 2 + 1024];
        long outputPos = 0;
        
        std::string current;
        for (long i = 0; i < inputSize; i++) {
            std::string next = current + static_cast<char>(inputData[i]);
            
            if (dictionary.count(next)) {
                current = next;
            } else {
                int code = dictionary[current];
                outputData[outputPos++] = (code >> 8) & 0xFF;
                outputData[outputPos++] = code & 0xFF;
                
                if (dictSize < 4096) {
                    dictionary[next] = dictSize++;
                }
                current = std::string(1, static_cast<char>(inputData[i]));
            }
        }
        
        if (!current.empty()) {
            int code = dictionary[current];
            outputData[outputPos++] = (code >> 8) & 0xFF;
            outputData[outputPos++] = code & 0xFF;
        }
        
        result.compressionTime = timer.stopTimer();
        result.originalSize = inputSize;
        result.compressedSize = outputPos;
        result.ratio = (double)outputPos / inputSize;
        result.success = writeFile(outputFile, outputData, outputPos);
        
        delete[] outputData;
    }
    catch (...) {
        std::cout << "Ошибка в LZW сжатии" << std::endl;
    }
    
    delete[] inputData;
    return result;
}

CompressionResult LZWCompressor::decompress(const char* inputFile, const char* outputFile) {
    Timer timer;
    CompressionResult result;
    result.success = false;
    
    unsigned char* inputData = nullptr;
    long inputSize = 0;
    if (!readFile(inputFile, &inputData, &inputSize)) {
        return result;
    }
    
    timer.startTimer();
    
    try {
        std::map<int, std::string> dictionary;
        for (int i = 0; i < 256; i++) {
            dictionary[i] = std::string(1, static_cast<char>(i));
        }
        
        int dictSize = 256;
        unsigned char* outputData = new unsigned char[1000000];
        long outputPos = 0;
        
        long inputPos = 0;
        int prevCode = -1;
        
        while (inputPos + 1 < inputSize) {
            int code = (inputData[inputPos] << 8) | inputData[inputPos + 1];
            inputPos += 2;
            
            std::string entry;
            
            if (dictionary.count(code)) {
                entry = dictionary[code];
            } else if (code == dictSize && prevCode != -1) {
                entry = dictionary[prevCode] + dictionary[prevCode][0];
            } else {
                break;
            }
            
            for (size_t j = 0; j < entry.size(); j++) {
                if (outputPos < 1000000) {
                    outputData[outputPos++] = entry[j];
                }
            }

            if (prevCode != -1 && dictSize < 4096) {
                dictionary[dictSize] = dictionary[prevCode] + std::string(1, entry[0]);
                dictSize++;
            }
            prevCode = code;
        }
        
        result.decompressionTime = timer.stopTimer();
        result.success = writeFile(outputFile, outputData, outputPos);
        
        delete[] outputData;
    }
    catch (...) {
        std::cout << "Ошибка в LZW распаковке" << std::endl;
    }
    
    delete[] inputData;
    return result;
}