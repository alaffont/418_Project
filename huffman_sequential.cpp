#include <iostream>
#include <string>
#include <queue>
#include <unordered_map>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <malloc.h>
#include <chrono>
#include <bits/stdc++.h>
using namespace std;

int decodeFd = 0;
char* decodedEncoding;
int position = 0;

// A Tree node
struct Node
{
	char ch;
	int freq;
	Node *left, *right;
};

// Function to allocate a new tree node
Node* getNode(char ch, int freq, Node* left, Node* right)
{
	Node* node = new Node();

	node->ch = ch;
	node->freq = freq;
	node->left = left;
	node->right = right;

	return node;
}

// Comparison object to be used to order the heap
struct comp
{
	bool operator()(Node* l, Node* r)
	{
		// highest priority item has lowest frequency
		return l->freq > r->freq;
	}
};

// traverse the Huffman Tree and store Huffman Codes
// in a map.
void encode(Node* root, string str,
			unordered_map<char, string> &huffmanCode)
{
	if (root == nullptr)
		return;

	// found a leaf node
	if (!root->left && !root->right) {
		huffmanCode[root->ch] = str;
	}

	encode(root->left, str + "0", huffmanCode);
	encode(root->right, str + "1", huffmanCode);
}

// traverse the Huffman Tree and decode the encoded string
void decode(Node* root, int &index, string str)
{
	if (root == nullptr) {
		return;
	}

	// found a leaf node
	if (!root->left && !root->right)
	{
		decodedEncoding[position++] = root->ch; 
		return;
	}

	index++;

	if (str[index] =='0')
		decode(root->left, index, str);
	else
		decode(root->right, index, str);
}

// Builds Huffman Tree and decode given input text
void buildHuffmanTree(string text)
{
	time_t start, end;
	//time starter for building huffman tree
	time(&start);
    ios_base::sync_with_stdio(false);
	// count frequency of appearance of each character
	// and store it in a map
	unordered_map<char, int> freq;
	for (char ch: text) {
		freq[ch]++;
	}

	// Create a priority queue to store live nodes of
	// Huffman tree;
	priority_queue<Node*, vector<Node*>, comp> pq;

	// Create a leaf node for each character and add it
	// to the priority queue.
	for (auto pair: freq) {
		pq.push(getNode(pair.first, pair.second, nullptr, nullptr));
	}

	// do till there is more than one node in the queue
	while (pq.size() != 1)
	{
		// Remove the two nodes of highest priority
		// (lowest frequency) from the queue
		Node *left = pq.top(); pq.pop();
		Node *right = pq.top();	pq.pop();

		// Create a new internal node with these two nodes
		// as children and with frequency equal to the sum
		// of the two nodes' frequencies. Add the new node
		// to the priority queue.
		int sum = left->freq + right->freq;
		pq.push(getNode('\0', sum, left, right));
	}

	// root stores pointer to root of Huffman Tree
	Node* root = pq.top();
	// traverse the Huffman Tree and store Huffman Codes
	// in a map. 
	unordered_map<char, string> huffmanCode;
	time(&end);
    double time_taken = double(end - start);
    cout << "Time taken to build Huffman Tree is : " << fixed
        << time_taken << setprecision(5);
    cout << " sec " << endl;
	//time starter for time it takes to encode file
	start = time(NULL);
	time(&start);
    ios_base::sync_with_stdio(false);
	encode(root, "", huffmanCode);

	cout << "Huffman Codes are :\n" << '\n';
	for (auto pair: huffmanCode) {
		cout << pair.first << " " << pair.second << '\n';
	}

	string str = "";
	for (char ch: text) {
		str += huffmanCode[ch];
	}

    cout << "Length of encoded string: " << str.length() << endl;

    const char *cstr = str.c_str();
    int encbits = str.length();
    char *rawenc = (char *)malloc(encbits / 8);
    if (!rawenc) abort();

    for (int i = 0; i < encbits; i++) {
        int rawenc_idx = i / 8;
        int rawinc_off = i % 8;
        char bit = !(31 - cstr[i]);
        rawenc[rawenc_idx] |= (bit << (7 - rawinc_off));
    }

    int outfd = open("huffman_output.bin", O_WRONLY | O_TRUNC | O_CREAT, 0666);
    write(outfd, rawenc, encbits / 8);
	// traverse the Huffman Tree again and this time
	// decode the encoded string
	time(&end);
    time_taken = double(end - start);
    cout << "Time taken to encode program is : " << fixed
    << time_taken << setprecision(5);
    cout << " sec " << endl;
	//timer starting for decoder
	start = time(NULL);
    time(&start);
    ios_base::sync_with_stdio(false);
    int index = -1;
	while (index < (int)str.size() - 2) {
		decode(root, index, str);
	}
	time(&end);
    time_taken = double(end - start);
    cout << "Time taken to decode is : " << fixed
        << time_taken << setprecision(5);
    cout << " sec " << endl;
	
}

// Huffman coding algorithm
int main()
{
    decodeFd = open("huffman_output.txt", O_WRONLY, O_TRUNC | O_CREAT);
    std::ifstream t("example.txt");
    std::stringstream buffer;
    buffer << t.rdbuf();
	buffer.seekg(0, ios::end);
	int size = buffer.tellg();
	buffer.seekg(0, ios::beg);
	decodedEncoding = (char*)malloc(sizeof(char) * size);
	buildHuffmanTree(buffer.str());
	write(decodeFd, decodedEncoding, size);
	return 0;
}
