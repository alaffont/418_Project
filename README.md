### Parallelizing Different Compression Algorithms Using GPU
## Tyler Tan and Alfonso Laffont

# Summary
We are going to parallelize different compression algorithms such as LZSS and huffman compression using both GPU and multi-core CPU. Then, we are going to perform a detailed analysis of both system’s performance and characteristics, compared to the sequential form. 

# Background

Due to the technological advancements in the world, data is becoming more and more important. Nowadays, data is both collected at a larger rate as well as stored for longer periods of time. With this massive influx of data, it is increasingly important to be able to store all that data, which is why compression algorithms are so important right now. However, a lot of compression algorithms are slow and take long periods of time, due to the sheer amount of data that is being processed. To help solve this problem, we are planning on parallelizing compression algorithms to reduce the time it takes to compress files.

To parallelize the compression algorithms, we want to parallelize the binary tree given the data inputs. We also want to parallelize the way we gather data by pipelining the workers and segmenting the file into chunks. That way, each worker is able to process data while one worker is reading the file, minimizing the bottleneck caused by reading the entire file. We can also parallelize the way we write back the compressed file by dividing the compressed result into chunks and having each worker take a chunk to write into the file. 

# The Challenge
A problem that we may encounter regarding parallelizing the different compression algorithms is that you can’t compress something that you haven’t fully read, so we are forced into reading the entire file we are compressing sequentially. However, there are some ways we can alleviate this problem. One method could be to pipeline the different workers so that a worker first reads a section of the file and then begins to immediately process it. That way, a second worker can be reading the file while the first worker is processing the first section it reads, maximizing the efficiency of the compression. 

# Resources
We do not need any special computational resources, but we will be using several articles and powerpoints found online in order to implement our parallelized version of compression algorithms. We also did some research on the basics of the different compression algorithms in order to better understand how they work, even without being parallelized. As for our code, we are going to start from scratch, without any code base. 
http://a1bert.kapsi.fi/Dev/pucrunch/packing.html
https://en.wikipedia.org/wiki/Huffman_coding
https://go-compression.github.io/algorithms/lzss/

# Goals and Deliverables
Goals that we plan to achieve are parallelizing huffman compression and LZSS compression using both GPU and CPU, in order to showcase differing speedups from both machines. We also plan to lower the bottleneck caused by having to read the file being compressed by implementing a streamlining/compression part. Something that we hope to achieve if everything goes well is to implement more unique compression algorithms, such as DEFLATE, a compression algorithm that merges LZSS and huffman, or lossy compression algorithms. Another thing we hope to be able to work on is parallelizing a few decompression algorithms. 
	The type of demo we plan to showcase at the final presentation time are different speedup graphs showcasing the speedup we achieve using GPU vs CPU for many types of algorithms as well as how fast we were able to compress huge amounts of data (such as the entire english wikipedia database). 
	

# Platform Choice
We are planning to use the CUDA language to parallelize the different compression algorithms since CUDA has faster downloads and readbacks to the GPU. Since compression algorithms are mainly I/O bound, doing everything in GPU will produce the fastest speedup.

# Schedule
Week 1: Implement sequential huffman and LZSS compression
Week 2: Week 3: Implement cuda version of compression algorithms
Week 4: Implement streaming/pipeline compression or another form of avoid I/O bottleneck
Week 5: Conduct Data Analysis on parallelized compression algorithms
Week 6: Fine tune and pivot if necessary (last week depends on results of previous weeks)




