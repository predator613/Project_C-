#include "lzss.h"
#include "../compression.h"
#include <cstdint>
#include <fstream>
#include <algorithm>
#include <cstring>

struct LZSSConfig {
    int window_size = 4096;
    int lookahead_size = 18;
    int min_match_length = 3;
};

class LZSSCompressorImpl {
private:
    LZSSConfig config;

    int findBestMatch(const uint8_t* data, int data_size, int current_pos, int& match_length) {
        int best_offset = 0;
        match_length = 0;
        
        int search_start = std::max(0, current_pos - config.window_size);
        int max_possible_length = std::min(config.lookahead_size, data_size - current_pos);
        
        if (max_possible_length < config.min_match_length) {
            return 0;
        }
        
        for (int i = search_start; i < current_pos; ++i) {
            int current_length = 0;
            
            while (current_length < max_possible_length && 
                   (i + current_length) < current_pos && 
                   data[i + current_length] == data[current_pos + current_length]) {
                current_length++;
            }
            
            if (current_length > match_length && current_length >= config.min_match_length) {
                match_length = current_length;
                best_offset = current_pos - i;
                
                if (match_length == config.lookahead_size) {
                    break;
                }
            }
        }
        
        return best_offset;
    }

public:
    LZSSCompressorImpl(const LZSSConfig& cfg = LZSSConfig()) : config(cfg) {}

    uint8_t* compressData(const uint8_t* input_data, size_t input_size, size_t& output_size) {
        if (!input_data || input_size == 0) {
            output_size = 0;
            return nullptr;
        }
        
        size_t max_output_size = input_size * 2 + 1000;
        uint8_t* output_buffer = new uint8_t[max_output_size];
        bool* flags = new bool[input_size];
        
        size_t output_pos = 0;
        size_t flags_count = 0;
        
        int i = 0;
        int input_size_int = static_cast<int>(input_size);
        
        while (i < input_size_int) {
            int match_length = 0;
            int offset = 0;
            
            if (input_size_int - i >= config.min_match_length) {
                offset = findBestMatch(input_data, input_size_int, i, match_length);
            }
            
            if (offset > 0 && match_length >= config.min_match_length) {
                flags[flags_count++] = true;
                
                uint16_t encoded = ((offset - 1) << 4) | (match_length - config.min_match_length);
                output_buffer[output_pos++] = static_cast<uint8_t>((encoded >> 8) & 0xFF);
                output_buffer[output_pos++] = static_cast<uint8_t>(encoded & 0xFF);
                
                i += match_length;
            } else {
                flags[flags_count++] = false;
                output_buffer[output_pos++] = input_data[i];
                i++;
            }
        }
        
        size_t final_output_size = 6 + (flags_count + 7) / 8 + output_pos;
        uint8_t* final_output = new uint8_t[final_output_size];
        size_t final_pos = 0;
        
        uint32_t original_size = static_cast<uint32_t>(input_size);
        final_output[final_pos++] = (original_size >> 24) & 0xFF;
        final_output[final_pos++] = (original_size >> 16) & 0xFF;
        final_output[final_pos++] = (original_size >> 8) & 0xFF;
        final_output[final_pos++] = original_size & 0xFF;

        uint16_t flag_bytes = static_cast<uint16_t>((flags_count + 7) / 8);
        final_output[final_pos++] = static_cast<uint8_t>((flag_bytes >> 8) & 0xFF);
        final_output[final_pos++] = static_cast<uint8_t>(flag_bytes & 0xFF);

        for (size_t j = 0; j < flags_count; j += 8) {
            uint8_t flag_byte = 0;
            for (int k = 0; k < 8 && (j + k) < flags_count; ++k) {
                if (flags[j + k]) {
                    flag_byte |= (1 << (7 - k));
                }
            }
            final_output[final_pos++] = flag_byte;
        }

        for (size_t j = 0; j < output_pos; j++) {
            final_output[final_pos++] = output_buffer[j];
        }
        
        output_size = final_pos;
        
        delete[] output_buffer;
        delete[] flags;
        
        return final_output;
    }

    uint8_t* decompressData(const uint8_t* compressed_data, size_t compressed_size, size_t& output_size) {
        if (!compressed_data || compressed_size < 8) {
            output_size = 0;
            return nullptr;
        }
        
        uint32_t original_size = (static_cast<uint32_t>(compressed_data[0]) << 24) | 
                                (static_cast<uint32_t>(compressed_data[1]) << 16) | 
                                (static_cast<uint32_t>(compressed_data[2]) << 8) | 
                                static_cast<uint32_t>(compressed_data[3]);
        
        uint16_t flag_bytes = (static_cast<uint16_t>(compressed_data[4]) << 8) | static_cast<uint16_t>(compressed_data[5]);
        
        if (6 + flag_bytes > compressed_size) {
            output_size = 0;
            return nullptr;
        }
        
        uint8_t* output = new uint8_t[original_size];
        size_t output_pos = 0;

        size_t flags_count = flag_bytes * 8;
        bool* flags = new bool[flags_count];
        
        size_t flag_data_start = 6;
        for (size_t i = 0; i < flag_bytes; ++i) {
            uint8_t flag_byte = compressed_data[flag_data_start + i];
            for (int j = 7; j >= 0; --j) {
                size_t flag_index = i * 8 + (7 - j);
                if (flag_index < flags_count) {
                    flags[flag_index] = (flag_byte >> j) & 1;
                }
            }
        }
        
        size_t data_pos = flag_data_start + flag_bytes;
        size_t flag_index = 0;
        
        while (output_pos < original_size && flag_index < flags_count && data_pos < compressed_size) {
            if (!flags[flag_index]) {
                output[output_pos++] = compressed_data[data_pos];
                data_pos++;
                flag_index++;
            } else {
                if (data_pos + 1 >= compressed_size) break;
                
                uint16_t encoded = (static_cast<uint16_t>(compressed_data[data_pos]) << 8) | static_cast<uint16_t>(compressed_data[data_pos + 1]);
                int offset = (encoded >> 4) + 1;
                int length = (encoded & 0x0F) + config.min_match_length;
                
                if (offset > output_pos) {
                    data_pos += 2;
                    flag_index++;
                    continue;
                }
                
                size_t copy_pos = output_pos - offset;
                for (int i = 0; i < length && output_pos < original_size; ++i) {
                    if (copy_pos + i < output_pos) {
                        output[output_pos++] = output[copy_pos + i];
                    } else {
                        break;
                    }
                }
                
                data_pos += 2;
                flag_index++;
            }
        }
        
        output_size = output_pos;
        delete[] flags;
        
        return output;
    }
};

bool readFileData(const char* filename, uint8_t** data, size_t* size) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    file.seekg(0, std::ios::end);
    std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    if (file_size <= 0) {
        return false;
    }
    
    *data = new uint8_t[static_cast<size_t>(file_size)];
    if (!file.read(reinterpret_cast<char*>(*data), file_size)) {
        delete[] *data;
        return false;
    }
    
    *size = static_cast<size_t>(file_size);
    return true;
}

bool writeFileData(const char* filename, const uint8_t* data, size_t size) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    if (data && size > 0) {
        file.write(reinterpret_cast<const char*>(data), static_cast<std::streamsize>(size));
    }
    
    bool success = file.good();
    file.close();
    return success;
}

const char* LZSSCompressor::getName() const {
    return "LZSS";
}

CompressionResult LZSSCompressor::compress(const char* inputFile, const char* outputFile) {
    CompressionResult result;
    Timer timer;
    
    timer.startTimer();
    
    LZSSCompressorImpl compressor;
    size_t original_size = 0, compressed_size = 0;
    uint8_t* input_data = nullptr;
    uint8_t* compressed_data = nullptr;
    
    if (!readFileData(inputFile, &input_data, &original_size)) {
        result.success = false;
        return result;
    }
    
    compressed_data = compressor.compressData(input_data, original_size, compressed_size);
    
    if (compressed_data && writeFileData(outputFile, compressed_data, compressed_size)) {
        result.success = true;
        result.originalSize = static_cast<long>(original_size);
        result.compressedSize = static_cast<long>(compressed_size);
        result.ratio = static_cast<double>(compressed_size) / original_size;
    } else {
        result.success = false;
    }
    
    result.compressionTime = timer.stopTimer();
    
    delete[] input_data;
    delete[] compressed_data;
    
    return result;
}

CompressionResult LZSSCompressor::decompress(const char* inputFile, const char* outputFile) {
    CompressionResult result;
    Timer timer;
    
    timer.startTimer();
    
    LZSSCompressorImpl compressor;
    size_t compressed_size = 0, decompressed_size = 0;
    uint8_t* compressed_data = nullptr;
    uint8_t* decompressed_data = nullptr;
    
    if (!readFileData(inputFile, &compressed_data, &compressed_size)) {
        result.success = false;
        return result;
    }
    
    decompressed_data = compressor.decompressData(compressed_data, compressed_size, decompressed_size);
    
    if (decompressed_data && writeFileData(outputFile, decompressed_data, decompressed_size)) {
        result.success = true;
        result.originalSize = static_cast<long>(decompressed_size);
        result.compressedSize = static_cast<long>(compressed_size);
        result.ratio = static_cast<double>(compressed_size) / decompressed_size;
    } else {
        result.success = false;
    }
    
    result.decompressionTime = timer.stopTimer();
    
    delete[] compressed_data;
    delete[] decompressed_data;
    
    return result;
}