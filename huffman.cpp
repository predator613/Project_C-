#include "huffman.h"
#include <map>
#include <queue>
#include <string>

struct HuffmanNode {
    unsigned char data;
    int freq;
    HuffmanNode *left, *right;
    HuffmanNode(unsigned char d, int f) : data(d), freq(f), left(nullptr), right(nullptr) {}
};

struct Compare {
    bool operator()(HuffmanNode* a, HuffmanNode* b) {
        return a->freq > b->freq;
    }
};

const char* HuffmanCompressor::getName() const { 
    return "Huffman"; 
}

void buildCodes(HuffmanNode* node, std::string code, std::map<unsigned char, std::string>& codes) {
    if (!node) return;
    if (!node->left && !node->right) {
        codes[node->data] = code;
    }
    buildCodes(node->left, code + "0", codes);
    buildCodes(node->right, code + "1", codes);
}

void deleteTree(HuffmanNode* node) {
    if (!node) return;
    deleteTree(node->left);
    deleteTree(node->right);
    delete node;
}

CompressionResult HuffmanCompressor::compress(const char* inputFile, const char* outputFile) {
    Timer timer;
    CompressionResult result; 
    
    unsigned char* inputData;
    long inputSize;
    if (!readFile(inputFile, &inputData, &inputSize)) return result;
    
    timer.startTimer();
    
    int freq[256] = {0};
    for (long i = 0; i < inputSize; i++) {
        freq[inputData[i]]++;
    }
    
    std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, Compare> pq;
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            pq.push(new HuffmanNode(static_cast<unsigned char>(i), freq[i]));
        }
    }
    
    while (pq.size() > 1) {
        HuffmanNode* left = pq.top(); pq.pop();
        HuffmanNode* right = pq.top(); pq.pop();
        HuffmanNode* parent = new HuffmanNode(0, left->freq + right->freq);
        parent->left = left;
        parent->right = right;
        pq.push(parent);
    }
    
    if (pq.empty()) {
        delete[] inputData;
        return result;
    }
    
    HuffmanNode* root = pq.top();
    std::map<unsigned char, std::string> codes;
    buildCodes(root, "", codes);
    
    std::string encoded = "";
    for (long i = 0; i < inputSize; i++) {
        encoded += codes[inputData[i]];
    }
    
    long encodedSize = (encoded.length() + 7) / 8;
    unsigned char* outputData = new unsigned char[encodedSize + 1024];
    long pos = 0;
    

    for (int i = 0; i < 256; i++) {
        outputData[pos++] = (freq[i] >> 24) & 0xFF;
        outputData[pos++] = (freq[i] >> 16) & 0xFF;
        outputData[pos++] = (freq[i] >> 8) & 0xFF;
        outputData[pos++] = freq[i] & 0xFF;
    }
    
    for (long i = 0; i < encoded.length(); i += 8) {
        unsigned char byte = 0;
        for (int j = 0; j < 8; j++) {
            if (i + j < encoded.length() && encoded[i + j] == '1') {
                byte |= (1 << (7 - j));
            }
        }
        outputData[pos++] = byte;
    }
    
    result.compressionTime = timer.stopTimer();
    result.originalSize = inputSize;
    result.compressedSize = pos;
    result.ratio = (double)pos / inputSize;
    result.success = writeFile(outputFile, outputData, pos);
    
    delete[] inputData;
    delete[] outputData;
    deleteTree(root);
    return result;
}

CompressionResult HuffmanCompressor::decompress(const char* inputFile, const char* outputFile) {
    Timer timer;
    CompressionResult result;
    
    unsigned char* inputData;
    long inputSize;
    if (!readFile(inputFile, &inputData, &inputSize)) return result;
    
    timer.startTimer();
    
    int freq[256];
    long pos = 0;
    for (int i = 0; i < 256; i++) {
        freq[i] = (inputData[pos] << 24) | (inputData[pos+1] << 16) | (inputData[pos+2] << 8) | inputData[pos+3];
        pos += 4;
    }
    
    std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, Compare> pq;
    for (int i = 0; i < 256; i++) {
        if (freq[i] > 0) {
            pq.push(new HuffmanNode(static_cast<unsigned char>(i), freq[i]));
        }
    }
    
    while (pq.size() > 1) {
        HuffmanNode* left = pq.top(); pq.pop();
        HuffmanNode* right = pq.top(); pq.pop();
        HuffmanNode* parent = new HuffmanNode(0, left->freq + right->freq);
        parent->left = left;
        parent->right = right;
        pq.push(parent);
    }
    
    if (pq.empty()) {
        delete[] inputData;
        return result;
    }
    
    HuffmanNode* root = pq.top();
    

    unsigned char* outputData = new unsigned char[1000000];
    long outputPos = 0;
    HuffmanNode* current = root;
    
    for (long i = pos; i < inputSize; i++) {
        for (int j = 7; j >= 0; j--) {
            if (inputData[i] & (1 << j)) {
                current = current->right;
            } else {
                current = current->left;
            }
            
            if (!current->left && !current->right) {
                outputData[outputPos++] = current->data;
                current = root;
            }
        }
    }
    
    result.decompressionTime = timer.stopTimer();
    result.originalSize = outputPos; 
    result.compressedSize = inputSize; 
    result.ratio = (double)inputSize / outputPos; 
    result.success = writeFile(outputFile, outputData, outputPos);
    
    delete[] inputData;
    delete[] outputData;
    deleteTree(root);
    return result;
}