#include "compression.h"
#include <iostream>

bool readFile(const char* filename, unsigned char** data, long* size) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cout << "Ошибка: не могу открыть файл " << filename << std::endl;
        return false;
    }
    
    file.seekg(0, std::ios::end);
    *size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    if (*size <= 0) {
        std::cout << "Ошибка: файл пустой" << std::endl;
        return false;
    }
    
    *data = new unsigned char[*size];
    file.read(reinterpret_cast<char*>(*data), *size);
    file.close();
    return true;
}

bool writeFile(const char* filename, unsigned char* data, long size) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cout << "Ошибка: не могу записать файл " << filename << std::endl;
        return false;
    }
    file.write(reinterpret_cast<char*>(data), size);
    file.close();
    return true;
}