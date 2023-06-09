// Parallel Implementation of LZSS with no wrapping
// This implemenetation is based on the approach given 
// given in sequential file written by Alfonso Laffont
// ########################################################################################

// Current Task: 
// In the search phase, we will use CUDA threads to search for matching substrings in parallel
// Will also launch a kernel In the encode phase.

// Why not CUDA streams?
// Using CUDA streams to overlap the data transfers with the computation
// CUDA streams allow you to execute multiple kernels concurrently on the GPU
// and overlap data transfers with the computation

// If we look at runtime, reading from disk is really quick and compression is really slow.
// So. It turns out that we aren't being bottlenecked by I/O and should thus not use streams

// Instead, focus on what is limitting compression > Algorithmic approach might not be ideal!


#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include <bitset>
//measure how long code takes
#include <chrono>
using namespace std;

// GLOBAL VARIABLES
#define MATCH_LENGTH_MASK (15)
#define WINDOW_SIZE (4095)
#define GRANULARITY (WINDOW_SIZE+1)

// ascii to char
void print_char(int v){
    std::cout << (char)v;
}
// get match distance using bit operations
uint16_t get_distance(uint16_t input){
    return (input >> 4); 
}

// get match length using bit operations
__host__ __device__ uint16_t get_length(uint16_t input) {
    return (input & 0xF);
}

void print_string(std::vector<unsigned char> s1, int start, int end){
    for(int i = start; i < end; i++){
        std::cout << s1.at(i);
    }
    std::cout << std::endl;
}

bool are_vectors_equal(std::vector<unsigned char> s1, std::vector<unsigned char> s2){
    if(s1.size() != s2.size()){
        printf("Strings aren't equal length\n");
        return 0;
    }

    for(int i = 0; i < s1.size(); i++){
        if(s1.at(i) != s2.at(i)){
            printf("string values mismatch\n");
            return 0;
        }
    }
    printf("Strings are equal!\n");
    return 1;
}

__global__ void find_longest_match_kernel(unsigned char *data, int i, uint16_t *longest_match_array, int len) {
    
    uint orig_i = i;
    uint cur = threadIdx.x + max(0, i - WINDOW_SIZE);
    uint16_t orig_cur = cur;
    // create the struct for storing the longest word we are currently look at
    uint length_of_match = 0;
    uint longest_match = 0;
    uint start = 0;
    while(cur < i && i < len && length_of_match < MATCH_LENGTH_MASK){
        // we have a match
        if(data[cur] == data[i]){
            cur++;
            i++;
            length_of_match++;
        }else{
            // we have a new longest match
            if (longest_match < length_of_match){
                longest_match = length_of_match;
                length_of_match = 0;
                start = cur;
            }
            break;
        }
    }
    // dont compress size 0 or 1
    if (longest_match < 2){
        longest_match_array[orig_cur] = 0;
    }
    // compress case
    else{
        uint16_t ret = 0;
        // bit shifting based on header of file
        ret = ((orig_i - ((start - longest_match))) << 4) | (longest_match);
        longest_match_array[orig_cur] = ret;
    }
}

// use a reduction function to find the max value in an array
// set the first element to this max
// we only want to base our max on size rather than distance
__global__ void find_max_reduction_kernel(uint16_t* data, int data_size){
    // we launch one thread per index
    int idx = threadIdx.x;
    if (idx < data_size){
        for(int stride = 1; stride < data_size; stride *= 2) {
            if (idx % (2*stride) == 0) {
                uint16_t lhs = get_length(data[idx]);
                uint16_t rhs = get_length(data[idx + stride]);
                data[idx] = lhs < rhs ? data[idx + stride] : data[idx];
            }
            __syncthreads();
        }
    }
}

uint16_t seq_find_longest_match(std::vector<unsigned char> data, int i){
    uint orig_i = i;
    uint cur = max(0, i - WINDOW_SIZE);
    // create the struct for storing the longest word we are currently look at
    uint length_of_match = 0;
    uint longest_match = 0;
    uint start = 0;
    while(cur < i && i < data.size()){
        if(data.at(i) == data.at(cur) && length_of_match <= MATCH_LENGTH_MASK){
            // increment i and cur since we have a match
            i++;
            cur++;
            length_of_match++;
        }else{
            // check if longest string so far
            if (longest_match < length_of_match){
                // new length
                start = cur;
                longest_match = length_of_match;
                // break early if we find max length match
                if (longest_match >= MATCH_LENGTH_MASK)
                    break;
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
    ret = ((orig_i - ((start - longest_match))) << 4) | (longest_match);

    return ret;
}


// keep an array of longest matches. 
// have each index select the length of the match  
uint16_t find_longest_match(unsigned char *d_data, int input_size, int i){
    int number_of_threads = WINDOW_SIZE + 1;

    uint16_t *device_longest_match_array;
    uint longest_match_array_length = min(WINDOW_SIZE, i + 1);
    // set things to 0 if not useful so no need to worry
    cudaMalloc(&device_longest_match_array, 2 * longest_match_array_length);
    // invoke our kernel
    find_longest_match_kernel<<<1, min(i+1, number_of_threads)>>>(d_data, i, device_longest_match_array, input_size);
    // find max in using interleave
    find_max_reduction_kernel<<<1, min(i+1, number_of_threads)>>>(device_longest_match_array, input_size);
    // alloc host mem
    uint16_t res;
    cudaMemcpy(&res, device_longest_match_array, 2, cudaMemcpyDeviceToHost);

    // only need to copy one item
    // cudaMemcpy(host_longest_match_array, device_longest_match_array, 2*longest_match_array_length, cudaMemcpyDeviceToHost);

    // cudaFree(device_longest_match_array);

    return res;
}


// unsigned char is the representation of bytes that we will be using
std::vector<unsigned char> compress(unsigned char *d_data, std::vector<unsigned char> data, std::vector<bool> &compress_flags, int input_size){
    int i = 0;
    std::vector<unsigned char> output_buffer;
    // loop through all char in text
    while (i < input_size){
        uint16_t longest_match;
        // granularity tells us if we should run code in parallel or sequentially
        if (i > GRANULARITY){
            longest_match = seq_find_longest_match(data, i);
        }else{
            longest_match = find_longest_match(d_data, input_size, i);
        }
        uint16_t match_length = get_length(longest_match); 
        // item needs to be longer than 2 in order to save data
        if(match_length > 2){
            // 12 bytes = distance and 4 bytes is length
            uint16_t match_distance = get_distance(longest_match);
            // uint16_t match_length = get_length(longest_match); 
            // indicate that we are compressing
            compress_flags[i] = 1;
            output_buffer.push_back((longest_match >> 8) & 0xFF);
            output_buffer.push_back(longest_match & 0xFF);

            i += match_length;
        }else{
            output_buffer.push_back(data.at(i));
            i += 1;
        }
    }
    return output_buffer;
}

// decompress unsigned char vector streams 
// actual structure of data compression is described in header
std::vector<unsigned char> decompress(std::vector<unsigned char> data, std::vector<bool> &compress_flags){
    int compress_index = 0;
    uint data_index = 0;
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

            for(int i = 0; i < match_length; i++){
                output.push_back(output[((compress_index - match_distance) + i)]);
            }
            // two bytes for compressed
            data_index += 2;
            compress_index += match_length;
        }
        // item isn't compressed
        else{
            output.push_back(data[data_index]);
            // one byte for non compressed
            data_index++;
            compress_index++;
        }
    }
    return output;
}

int main(int argc, char *argv[]) {
    std::ifstream inFile;
    inFile.open(argv[1]); 
    
    std::stringstream strStream;
    auto read_file_start_time = std::chrono::high_resolution_clock::now();
    //read the file
    strStream << inFile.rdbuf(); 
    std::string s = strStream.str(); 
    // text input
    vector<unsigned char> input(s.begin(), s.end());

    // copy data into GPU
    uint input_size = ((int)input.size());
    unsigned char *d_data;
    cudaMalloc(&d_data, input_size);
    cudaMemcpy(d_data, input.data(), input_size, cudaMemcpyHostToDevice);

    auto read_file_end_time = std::chrono::high_resolution_clock::now();

    // compression flags all init at 0
    vector<bool> compress_flags(input.size(), 0);
    
    auto compression_start_time = std::chrono::high_resolution_clock::now();
    auto compressed_buf = compress(d_data, input, compress_flags, input_size);
    auto compression_end_time = std::chrono::high_resolution_clock::now();
    cudaFree(d_data);

    auto decompression_start_time = std::chrono::high_resolution_clock::now();
    auto output = decompress(compressed_buf, compress_flags);
    auto decompression_end_time = std::chrono::high_resolution_clock::now();

    // verify results
    bool res = are_vectors_equal(output, input);

    // save time durations
    auto compression_duration = (std::chrono::duration_cast<std::chrono::microseconds>(compression_end_time - compression_start_time).count()/pow(10,6));
    auto decompression_duration = (std::chrono::duration_cast<std::chrono::microseconds>(decompression_end_time - decompression_start_time).count()/pow(10,6));
    auto read_duration = (std::chrono::duration_cast<std::chrono::microseconds>(read_file_end_time - read_file_start_time).count()/pow(10,6));
    // write to output file

    ofstream myfile;
    myfile.open("output.txt");

    // report duartion inside outfile
    myfile << "Reading Input & Copying to GPU took: " << read_duration << " seconds" << std::endl;
    myfile << "Compression took: " << compression_duration << " seconds" << std::endl;
    myfile << "Decompression took: " << decompression_duration << " seconds" << std::endl;
    myfile.close();
    
    return 1;
}
