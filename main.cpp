#include "compression.h"
#include "Huffman_Elya/huffman.h"
#include "LZSS_Masha/lzss.h"
#include "LZ77_Diana/lz77.h"
#include "LZ78_Miri/lz78.h"
#include "LZW_Malika/lzw.h"
#include <iostream>
#include <iomanip>
#include <cstdio>

int main() {
    std::cout << "=== СИСТЕМА СРАВНЕНИЯ АЛГОРИТМОВ СЖАТИЯ ===\n\n";
    
    char inputFile[256];
    std::cout << "Введите имя файла для сжатия: ";
    std::cin >> inputFile;
    
    Compressor* compressors[] = {
        new HuffmanCompressor(),
        new LZSSCompressor(),
        new LZ77Compressor(),
        new LZ78Compressor(),
        new LZWCompressor()
    };
    
    const int numCompressors = sizeof(compressors) / sizeof(compressors[0]);
    CompressionResult results[numCompressors];
    
    std::cout << "\n=== ПРОЦЕСС СЖАТИЯ ===\n";
    
    for (int i = 0; i < numCompressors; i++) {
        std::cout << "\n--- " << compressors[i]->getName() << " ---\n";
        
        char compressedFile[256];
        sprintf(compressedFile, "compressed_%s.bin", compressors[i]->getName());
        
        char decompressedFile[256];
        sprintf(decompressedFile, "decompressed_%s.bin", compressors[i]->getName());

        results[i] = compressors[i]->compress(inputFile, compressedFile);
        if (results[i].success) {
            std::cout << "Сжатие: " << std::fixed << std::setprecision(2) 
                      << results[i].compressionTime << " мс\n";
            
            CompressionResult decompResult = compressors[i]->decompress(compressedFile, decompressedFile);
            results[i].decompressionTime = decompResult.decompressionTime;
            
            std::cout << "Распаковка: " << results[i].decompressionTime << " мс\n";
        } else {
            std::cout << "Ошибка сжатия!\n";
        }
    }
    
    std::cout << "\n=== РЕЗУЛЬТАТЫ СРАВНЕНИЯ ===\n";
    std::cout << std::left << std::setw(10) << "Алгоритм" 
              << std::setw(12) << "Сжатие(мс)" 
              << std::setw(12) << "Распак.(мс)"
              << std::setw(12) << "Исходный" 
              << std::setw(12) << "Сжатый" 
              << std::setw(10) << "Коэфф." 
              << std::setw(10) << "Эфф.%\n";
    std::cout << "----------------------------------------------------------------------------\n";
    
    for (int i = 0; i < numCompressors; i++) {
        if (results[i].success) {
            double efficiency = (1.0 - results[i].ratio) * 100.0;
            std::cout << std::left << std::setw(10) << compressors[i]->getName()
                      << std::setw(12) << std::fixed << std::setprecision(2) << results[i].compressionTime
                      << std::setw(12) << results[i].decompressionTime
                      << std::setw(12) << results[i].originalSize
                      << std::setw(12) << results[i].compressedSize
                      << std::setw(10) << std::setprecision(3) << results[i].ratio
                      << std::setw(10) << std::setprecision(1) << efficiency << "%\n";
        }
    }
    
    int bestIndex = -1;
    double bestEfficiency = -100.0;
    
    for (int i = 0; i < numCompressors; i++) {
        if (results[i].success) {
            double efficiency = (1.0 - results[i].ratio) * 100.0;
            if (efficiency > bestEfficiency) {
                bestEfficiency = efficiency;
                bestIndex = i;
            }
        }
    }
    
    if (bestIndex != -1) {
        std::cout << "\nЛУЧШИЙ АЛГОРИТМ: " << compressors[bestIndex]->getName() 
                  << " (эффективность: " << std::fixed << std::setprecision(1) << bestEfficiency << "%)\n";
    }
    
    for (int i = 0; i < numCompressors; i++) {
        delete compressors[i];
    }
    
    return 0;
}