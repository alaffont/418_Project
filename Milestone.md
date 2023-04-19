### Updated Schedule
4/19-4/24: Parallelize huffman compression as well as implementing pipelining (Tyler) <br /> 
4//24-4/30: Parallelize LZSS compression as well as implementing pipelining (Alfonso) <br /> 
5/1: Run and take measurements of sequential code and parallelized code (Both) <br /> 
5/2-5/4: Work on posterboard graphs, documentation and report(Both) <br /> 

### Work Update
	So far we have finished implementing both sequential versions of LZSS compression as well as Huffman compression, although they have both taken `<br>`
significantly longer than expected. However, we expected this step to take the longest amount of time to code up, since we didn’t use any of the many sources `<br>`
online for sequential code versions of both compressions.The reason we didn’t use any online source for this code is because we wanted to write our own version `<br>`
in order to make it easier for us to parallelize. `<br>`

	In addition, we have built tools that we will use when debugging our parallel version. This will make our parallelization go quicker. `<br>`

	As for our planned out goals and deliverables, we firmly believe that we will be able to meet our original expectations since we have been able to `<br>`
stay on pace with our first schedule. However, it feels unlikely that we will be able to exceed our expectations and work on other compression algorithms `<br>`
since writing up the sequential versions of these compression algorithms takes up a significant amount of time. The reason is because while these sequential `<br>`
compression algorithms are available online we have to redo them in a way that allows us to easily parallelize and maximize the GPU’s effectiveness. So, our `<br>`
goals remain relatively the same, as we plan to complete data analysis on Huffman compression and LZSS compression, parallelized vs sequential vs pipelined. `<br>`

	For our posterboard, we plan on showing graphs displaying the differences in speedup between sequential, parallelized, and parallelized with lower `<br>`
I/O bound bottleneck. We also plan to run our code beforehand on the entire English dictionary or something of similar size, in order to showcase the `<br>`
speed of the compression algorithms. We also plan to display specific parts of where we parallelized the compression algorithms and why we chose `<br>`
those specific parts to parallelize. `<br>`

	A lot of our concerns have been implementing sequential versions of the code, since we not only have to write our own sequential version but also plan to parallelize it in the future. For all of our assignments, most of the sequential code is given to us and we just have to parallelize it, but we are worried our sequential code might not be easily parallelizable and will cause us to have poor results. 

