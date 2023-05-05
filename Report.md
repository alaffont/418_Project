Parallel Compression
Tyler Tan, Alfonso Laffont

Summary
The goal of our project was to implement both Huffman compression and LZSS compression algorithms using CUDA. The project deliverables include a sequential and parallel implementation of Huffman and LZSS compression algorithms including graphs/data analysis describing the work balance, tuning of parameters, and tradeoffs between parallelization and compression amount. We were able to achieve a close to 29x speedup for our huffman code compared to our sequential huffman code by launching pragma omp threads to traverse and build the huffman tree, allowing for significant speedups for encoding and decoding. We found that LZSS was able to achieve a greater than 8x speedup compared to sequential code by launching one cuda thread for recursive lookup back and using granularity to decrease kernel overhead. 


Background
As the amount of data in the world increases, the requirements to store this data increase as well. So, we decided to implement compression algorithms and parallelize them. The two compression algorithms that we picked were LZSS and Huffman compression. These are some of the commonly used compression algorithms. Both of these algorithms are lossless compression algorithms which means that they compress data without any loss of information or reduction in quality. This is an important feature since it allows for no consequences of compression.

LZSS compression works by referencing characters that have repeated characters rather than writing them again keeping the location of the match and the length of the match. 

The huffman compression algorithm compresses files by using less memory to represent characters that occur more frequently while using even more memory than usual to represent characters that occur less frequently. 







LZSS Compression

All inspiration came from a numberphile video I watched and I learned the functionality of the algorithm from the LZSS wikipedia page. The implementation is as follows: we are going to loop through each char and see if we’ve seen it in the past, if we have, we are going to find the longest match we have seen so far. If the match is longer than 2, we are going to store this match instead of the data.

Algorithmic Details

The structure of storing is as follows: uint16_t to store compressed data. The first four bytes represent the size of the match (asdf would be 4) and the next 12 bytes represent the distance from our current index to the start of our match referred to as window size. We will look into this again when we decode our compressed data. 

We want items to be larger than 2 bytes in order to compress because 2 unsigned characters are equal to 2 bytes which is the same as our compressed item which is 2 bytes. Anything smaller would be inflating the size of our compressed file instead of making it smaller. 

While encoding we keep a bit vector equal to the number of characters which will be the number of characters/8 bytes large. This allows us to know which items are compressed and which are not. We will loop through each compress byte, if our byte isn’t compressed we just return that byte. Otherwise, we uncompress it backwards by match distance and reading match length bytes. 

Parallel Implementation

The parallel approach works by launching one cuda thread for each previous character check for a match. Sequentially, we would loop for min(our current index, window size) since this is the most amount we can look backwards looking for a match. If we find a match, we iterate until the match is longer and find the largest one. 

In parallel, we are going to launch one thread for each index storing the size and distance of each inside an array. Then, we are going to use a reduce which finds the max optimally making sure to not just naively compare values but rather extracting match length since this is our criteria of importance. 

One area of optimization was to reduce the memcpy that occurs moving data from GPU to CPU. While the sequential code was laid out in such a way where our find function would access each portion of memory, doing so in a kernel would be highly inefficient. Instead, we move all the data once and access our desired sub portions. This greatly improves performance. 

Another area which had huge improvements was the use of granularity. For initial characters, we do not have much text to match against so launching cuda threads will have a huge overhead. By using sequential code when our window is small and using parallel code when our window is large, we got the best of both worlds. 


Trade Offs

This is a critical point where we see the max size we can represent is 2^4 - 1 = 15 characters and can look back at most 2^12 - 1 = 4095 characters. By keeping our compression item size a 2 bytes, we can either increase our window size which will in turn decrease our max size or decrease our window size which will increase our match size. If we increase window size, we have more opportunities to parallelize. However, a larger match size also means that we can compress our data more given that our file is sufficiently large. This tradeoff can be demonstrated by this graphic below: 



Our parallel performance can’t be seen on the graph because the values are so small but we can see that performance starts to taper off for our parallel code as our distance becomes smaller because we launch far fewer cuda threads. As we launch more cuda threads, we observe a faster speedup but the compression rate suffers as seen below. 


From these two graphs, we can see that the optimal compression size is when we let size be 2^4 = 16 and window size be 2^12 = 4096. While this may seem like hyper tuning to our data, it turns out that this holds more generally especially with text files. 

Performance:
To measure performance we read from disk, compress our file, and then decompress a file. For this test, we compress a txt file of Romeo and Juliet which can be found here. 






More specifically, we see the following performance for sequential compression



This is the performance we get from our parallel compression. 



Our file goes from 174,546 bytes to 81,394 bytes which is a 54% reduction in size. Results are verified with a string comparison of input and output string. There is an additional cost of 21,819 bytes in order to store a flag which marks which items are compressed and which are not. Including this we get a 48.1% reduction in our file size. 

Huffman Compression
The huffman compression algorithm has 3 main parts: Building the Huffman tree, encoding the inputted string using the huffman tree, and decoding the encoded bits that are produced. We only use a simple Node struct to represent the character and the frequency of it, having the children of the nodes total frequency be the frequency held.
struct Node
{
    char ch;
    int freq;
    Node *left, *right;
};

1) Building the tree
The way we build the huffman tree is to first create a priority queue that stores all of the characters in the huffman tree, creating internal nodes with a frequency equal to its 2 children node frequencies added together. We store non leaf-node characters as \0 or a NULL character, and each leaf stores one of the 256 ASCII characters. The more frequent characters will appear at higher levels of the tree while the more obscure characters will be found towards the bottom of the tree. 

2) Encoding the file
To encode the string given the huffman tree, we recursively call a function that starts at the root of the huffman tree and then as it moves left it adds a 0 to the encoding while adding a 1 to the encoding as it moves right. Then, when it finds a leaf node, it will store that binary code as the key for that character, and then after it is done building the key it goes through the string and sets each character to its corresponding huffman tree value. The encoded tree looks similar to the example below.

3) Decoding the file
To decode the binary file into the original non-compressed text, we recursively traverse the Huffman tree and create the decoding key, similar to encoding but essentially checking the key instead of adding to the key. If you see a 0 in the key, you move left in the tree. If you see a 1, you move right in the tree. Then, when you get to a leaf, set that section of bits to the appropriate character. Decoding naturally takes significantly longer time compared to encoding for larger files since you need to traverse the tree for each character in the original file, while for encoding you only need to traverse the huffman tree up to 256 times, once for each ASCII value, to determine the key.

Workload Distribution for Huffman Sequential

As you can see, there is not much inherent value at the start to parallelizing building the huffman tree since it is inherently sequential and will have lower computation times compared to huffman decoding. So, our starting approach to parallelize the huffman code is focused on the decoding. 

Approach: Sequential Version
For the sequential version, we started with an online github of huffman compression in c++ and then modified functions to better suit parallelization. We also modified the way we input and output our files to better store our results, as well as adding a timer to help set a benchmark for comparisons to future parallel versions.

Approach: Parallel Version
Given the previous workload distribution from encoding, decoding, and building the huffman tree, we decided to focus all of our attention on parallelizing the way we decode the huffman tree. We left most of the edited sequential code intact, primarily focusing on for loops and parallelizing those key points.

To parallelize decoding, we used #pragma omp for schedule(static) to launch several threads to traverse the huffman tree in order to decode faster. However, making just this change created a bunch of race conditions, as several threads were trying to write to the same position in the decoded text file which led to the file being all of the same characters. In order to fix this, we first tried making the index variable to the position of the decoded text private but that was not able to solve the entire problem. We ended up discovering a method called #pragma omp critical which forces threads to execute that snippet of code one at a time, which we only called if the decoding algorithm reached one of the huffman tree leafs. That way, it would not overly slow the threads and only execute one at a time if absolutely necessary. This allowed us to speedup decode by almost 30 times compared to the original decoding. 

Another section of the code we parallelized was the building of the huffman tree, since the workload distribution after parallelizing decoding led to building the huffman tree taking the longest time. To parallelize the building of the tree, we did dynamic scheduling to go through the text in order to build the frequency tree and atomically add to the frequency tree as you traverse the text file. The resulting workload distribution looks like this. 


Speedup
To measure our performance with the change of thread counts, we ran our parallel code against Romeo and Juliet while keeping track of the time in ms, which led us to achieve these results. These results showcase the reasoning why higher speedups occur with 16 threads compared to 8 threads since there are a lot of uncommon characters in the text file, while there are only a handful of really common characters. So, if the thread count is low, that would lead to the workload distribution between threads to be off balance since one thread could deal with most of the common characters while the other threads only have rare characters that are barely messed with for encoding or decoding. Since there are 256 characters possible, it makes sense that 16 threads was the ideal count, since that way the spread of the common characters is relatively even. 

Some things that we could have done to even increase the speedup would be to add granularity checks for the text file size, as we found that the sequential code was faster compared to the parallel code if the text file was super small (we used the first two lines of the famous Dr.Seuss book, green eggs and ham). Overall, we were limited in our potential for speedup since we are I/O bound, as reading the entire file will take a constant amount of time that cannot be parallelized. Another thing that limited our speedup for decoding was having to write back to memory. 

The reason we chose to use OpenMP instead of CUDA for huffman coding is because OpenMp is better for parallelizing using shared memory, which is much more difficult to implement in CUDA. Since the “calculations” we were doing for each of the threads were the same and relatively simple, we decided to use OpenMP. 


Work Distribution
Tyler <br>
Modifying existing huffman sequential code <br>
Huffman parallelization code <br>
Debugging <br>
Report <br>
<br><br>
Alfonso<br>
Sequential LZSS implementation <br>
Parallel LZSS implementation <br>
Debugging<br>
Report<br>
Resources
Ojala, P. (n.d.). Compression Basics. Compression basics. Retrieved May 5, 2023, from http://a1bert.kapsi.fi/Dev/pucrunch/packing.html 
Wikimedia Foundation. (2023, April 19). Huffman coding. Wikipedia. Retrieved April 24, 2023, from https://en.wikipedia.org/wiki/Huffman_coding 
LZSS. The Hitchhiker's Guide to Compression. (n.d.). Retrieved April 16, 2023, from https://go-compression.github.io/algorithms/lzss/ 
farhana786. (2020, October 12). Data-structure-Programs/Huffmancoding. GitHub. Retrieved April 20, 2023, from https://github.com/farhana786/Data-Structure-Programs/blob/main/HuffmanCoding 
https://www.openmp.org/spec-html/5.0/openmpsu104.html
https://www.smcm.iqfr.csic.es/docs/intel/compiler_c/main_cls/optaps/common/optaps_par_compat_libs_using.htm
https://ieeexplore.ieee.org/document/6413553
https://ieeexplore.ieee.org/document/8367376





