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
char cast_to_char(int v){
    return static_cast<char>(v);
}
// get match distance using bit operations
uint16_t get_distance(uint16_t input){
    return (input >> 4); 
}

// get match length using bit operations
uint16_t get_length(uint16_t input){
    return (input & 0xF);
}

uint16_t find_longest_match(std::vector<unsigned char> data, int i){
    int orig_i = i;
    int cur = max(0, i - WINDOW_SIZE);
    // create the struct for storing the longest word we are currently look at
    int length_of_match = 0;
    int longest_match = 0;
    int start = 0;
    while(cur < i){
        if(data.at(i) == data.at(cur)){
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
        }
    } 
    // 12 bytes = distance and 4 bytes is length
    if(longest_match < 2){
        return 0;
    }
    // use our custom way of storing length and start of match in 2 bytes
    uint16_t ret = start - longest_match; 
    ret = ret << 4;
    ret = ret & longest_match;
    return ret;
}

// unsigned char is the representation of bytes that we will be using
std::vector<unsigned char> compress(std::vector<unsigned char> data, std::vector<bool> compress_flags){
    int i = 0;
    std::vector<unsigned char> output_buffer;
    // loop through all char in text
    while (i < data.size()){
        uint16_t longest_match = find_longest_match(data, i);
        if(longest_match != 0){
            // 12 bytes = distance and 4 bytes is length
            uint16_t match_distance = get_distance(longest_match);
            uint16_t match_length = get_length(longest_match); 
            // indicate that we are compressing
            compress_flags[i] = 1;
            printf("distance: %d\n", match_distance);
            printf("length: %d\n", match_length);
            output_buffer.push_back(longest_match);
            i += match_length;
        }else{
            output_buffer.push_back(data.at(i));
            i += 1;
        }
    }

    return output_buffer;
}

int decompress(std::vector<unsigned char> data, std::vector<bool> compress_flags){
    return 0;
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
    std::vector<unsigned char> input(s.begin(), s.end());
    // compression flags all init at 0
    std::vector<bool> compress_flags(input.size(), 0);

    auto output_buf = compress(input, compress_flags);
    printf("%ld\n", input.size());
    printf("%ld\n", output_buf.size());

    return 1;
}