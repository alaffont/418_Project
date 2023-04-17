// reading a text file
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
using namespace std;


// GLOBAL VARIABLES
int MATCH_LENGTH_MASK = 0xF;
int WINDOW_SIZE = 0xFFF;
bool IS_MATCH_BIT = 1;

uint16_t find_longest_match(std::vector<unsigned char> data, int i){
    printf("size: %ld\n", data.size());
    // int buffer_start = min(data.size() / 4);
    // int buffer_end;

    return 0;
}

// unsigned char is the representation of bytes that we will be using
std::vector<unsigned char> compress(std::vector<unsigned char> data){

    int i = 0;
    std::vector<unsigned char> output_buffer;

    while (i < sizeof(data)){
        uint16_t longest_match = find_longest_match(data, i);
        if(longest_match != 0){
            // 12 bytes = distance and 4 bytes is length
            uint16_t match_distance = (longest_match >> 4); 
            uint16_t match_length = (longest_match & 0xF); 

            

            i += match_length;
        }else{
            i += 1;
        }
    }
    return output_buffer;
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

    std::vector<unsigned char> input(s.begin(), s.end());

    printf("string length: %d input length: %d\n", (int)str.length());
    

    return 1;
}