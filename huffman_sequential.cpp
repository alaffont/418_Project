#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <bitset>
using namespace std;

// GLOBAL VARIABLES
int MATCH_LENGTH_MASK = 16;
int WINDOW_SIZE = 4096;

// ascii to char
void print_char(int v){
    std::cout << (char)v;
}
// get match distance using bit operations
uint16_t get_distance(uint16_t input){
    return (input >> 4); 
}

// get match length using bit operations
uint16_t get_length(uint16_t input){
    return (input & 0xF);
}

bool are_vectors_equal(std::vector<unsigned char> s1, std::vector<unsigned char> s2){
    if(s1.size() != s2.size()){
        printf("Strings aren't equal length\n");
        return 0;
    }

    for(int i = 0; i < s1.size(); i++){
        if(s1.at(i) != s2.at(i)){
            printf("string values mismatch");
            return 0;
        }
    }
    printf("Strings are equal!\n");
    return 1;
}


uint16_t find_longest_match(std::vector<unsigned char> data, int i){
    uint orig_i = i;
    uint cur = max(0, i - WINDOW_SIZE);
    // create the struct for storing the longest word we are currently look at
    uint length_of_match = 0;
    uint longest_match = 0;
    uint start = 0;
    while(cur < orig_i & i < data.size()){
        if(data.at(i) == data.at(cur) & longest_match <= 16){
            // increment i and cur since we have a match
            i += 1;
            cur += 1;
            length_of_match += 1;
        }else{
            // check if longest string so far
            if (longest_match < length_of_match){
                // new length
                start = cur;
                longest_match = length_of_match;
            }
            // reset length of current match 
            length_of_match = 0;
            cur += 1;
            i = orig_i;
        }
    } 

    // 12 bytes = distance and 4 bytes is length
    if(longest_match < 2){
        return 0;
    }

    uint16_t ret = 0;
    // use our custom way of storing length and start of match in 2 bytes
    ret = (longest_match) | ((start - longest_match) << 4);

    return ret;
}


// unsigned char is the representation of bytes that we will be using
std::vector<unsigned char> compress(std::vector<unsigned char> data, std::vector<bool> &compress_flags){
    int i = 0;
    std::vector<unsigned char> output_buffer;
    // loop through all char in text
    while (i < data.size()){
        uint16_t longest_match = find_longest_match(data, i);
        // printf("3\n");
        uint16_t match_length = get_length(longest_match); 
        if(match_length > 2){
            // 12 bytes = distance and 4 bytes is length
            uint16_t match_distance = get_distance(longest_match);
            uint16_t match_length = get_length(longest_match); 
            // indicate that we are compressing
            compress_flags[i] = 1;
            // std::bitset<8> p1(longest_match & 0xFF);
            // std::bitset<8> p2((longest_match >> 8) & 0xFF);
            // std::bitset<16> w(longest_match);
            // std::cout << "p1: " << p1 << '\n';
            // std::cout << "p2: " << p2 << '\n';
            // std::cout << "whole: " << w << '\n';
            output_buffer.push_back((longest_match >> 8) & 0xFF);
            output_buffer.push_back(longest_match & 0xFF);
            // check if data is being packed correctly
            // printf("actual: ");
            // print_string(data, i, i + match_length);
            // printf("mine: ");
            // print_string(data, (match_distance), (match_distance) + match_length);
            // printf("\n");
            // printf("(%d,%d)\n", match_distance, match_length);
            // output_buffer.push_back(((unsigned char)(longest_match >> 4) && 0xff));
            i += match_length;
        }else{
            // printf("%d", i);
            // printf("%d", data.at(i));
            output_buffer.push_back(data.at(i));
            i += 1;
        }
    }
    return output_buffer;
}

std::vector<unsigned char> decompress(std::vector<unsigned char> data, std::vector<bool> &compress_flags){
    int compress_index = 0;
    int data_index = 0;
    std::vector<unsigned char> output;

    while(data_index < data.size()){
        // item is compressed
        // printf("%d\n", compress_flags.at(compress_index));
        if(compress_flags.at(compress_index) == 1){
            uint16_t p1 = data[data_index];
            uint16_t p2 = data[data_index+1];
            uint16_t compressed_data = ((p1 << 8) & 0xFF00) | p2;
            uint16_t match_distance = get_distance(compressed_data);
            uint16_t match_length = get_length(compressed_data); 
            // printf("(%d,%d)\n", match_distance, match_length);
            // output.push_back(40);
            // for (char c : std::to_string(match_distance)) {
            //     output.push_back(static_cast<int>(c));
            // }
            // output.push_back(44);
            // for (char c : std::to_string(match_length)){
            //     output.push_back(static_cast<int>(c));
            // }
            // output.push_back(41);
            for(int i = 0; i < match_length; i++){
                output.push_back(output[(match_distance)+i]);
            }
            // two bytes for compressed
            data_index += 2;
            compress_index += match_length;
        }
        // item isn't compressed
        else{
            output.push_back(data[data_index]);
            // one byte for non compressed
            data_index += 1;
            compress_index += 1;
        }
    }
    return output;
}


int main(int argc, char *argv[]) {
    // printf("%d",argc);
    // printf("%s", argv[1]);
    std::ifstream inFile;
    //open the input file
    inFile.open(argv[1]); 
    
    std::stringstream strStream;
    //read the file
    strStream << inFile.rdbuf(); 
    //str holds the content of the file
    std::string s = strStream.str(); 

    // text input
    vector<unsigned char> input(s.begin(), s.end());
    // compression flags all init at 0
    vector<bool> compress_flags(input.size(), 0);
    
    auto compressed_buf = compress(input, compress_flags);

    auto output = decompress(compressed_buf, compress_flags);

    bool res = are_vectors_equal(output, input);


    return 1;
}