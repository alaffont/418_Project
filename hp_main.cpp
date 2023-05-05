#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <malloc.h>
#include <iostream>
#include <string>
#include <queue>
#include <unordered_map>
#include <sstream>
#include <fstream>
using namespace std;
extern void buildHuffmanTree(string text);
int decodeFd = 0;
char *decodedEncoding = NULL;

int main(void) {
    decodeFd = open("huffman_p_output.txt", O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
    std::ifstream t("example.txt");
    std::stringstream buffer;
    buffer << t.rdbuf();
	buffer.seekg(0, ios::end);
	int size = buffer.tellg();
	buffer.seekg(0, ios::beg);
	decodedEncoding = (char*)malloc(sizeof(char) * size);
	buildHuffmanTree(buffer.str());
	(void)write(decodeFd, decodedEncoding, size);
	return 0;
}
