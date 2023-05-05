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
#include <cstdio>
#include <ctime>
#include <chrono>
#include <bits/stdc++.h>
#include <omp.h>

using namespace std;

int decodeFd = 0;
char* decodedEncoding;
int position = 0;
#pragma omp threadprivate(position)

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
        #pragma omp critical
        {
            decodedEncoding[position++] = root->ch;
        }
        return;
    }

    index++;

    if (str[index] == '0')
        decode(root->left, index, str);
    else
        decode(root->right, index, str);
}

// Builds Huffman Tree and decode given input text
void buildHuffmanTree(string text)
{
    std::clock_t startcputime = std::clock();
    // count frequency of appearance of each character
    // and store it in a map
    unordered_map<char, int> freq;
    #pragma omp parallel for shared(text, freq) schedule(dynamic)
    for (int i = 0; i < text.size(); ++i) {
        #pragma omp atomic
        freq[text[i]]++;
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
        Node *right = pq.top(); pq.pop();    
    // Create a new internal node with these two nodes as children
    // and with frequency equal to the sum of the two nodes'
    // frequencies. Add the new node to the priority queue.
    int sum = left->freq + right->freq;
    pq.push(getNode('\0', sum, left, right));
}

// root stores pointer to root of Huffman Tree
Node* root = pq.top();

// Traverse the Huffman Tree and store Huffman Codes
// in a map. Also prints them
unordered_map<char, string> huffmanCode;
encode(root, "", huffmanCode);

std::cout << "Huffman Codes are :\n" << '\n';
for (auto pair: huffmanCode) {
    std::cout << pair.first << " " << pair.second << '\n';
}
std::cout << '\n';

// Traverse the Huffman Tree again and this time
// decode the encoded string
decodedEncoding = (char*) malloc((text.size()+1)*sizeof(char));

#pragma omp parallel
{
    int index = -1;
    #pragma omp for schedule(static)
    for (int i = 0; i < (int)text.size(); i++) {
        if (text[i] == '0' || text[i] == '1') {
            decode(root, index, text);
        }
    }
}


decodedEncoding[position] = '\0';

//std::cout << "Decoded string is: " << decodedEncoding << '\n';
std::clock_t endcputime = std::clock();
std::cout << "CPU time used: "
          << 1000.0 * (endcputime-startcputime) / CLOCKS_PER_SEC
          << " ms\n";
}

int main()
{
    int fd = open("example.txt", O_RDONLY);
    if (fd == -1) {
        std::cerr << "Error: Could not open file\n";
        return 1;
    }
    struct stat st;
    fstat(fd, &st);
    int size = st.st_size;
    char* text = (char*) malloc((size+1)*sizeof(char));
    if (read(fd, text, size) == -1) {
        std::cerr << "Error: Could not read file\n";
        return 1;
        }
        text[size] = '\0';
        buildHuffmanTree(string(text));
        free(text);
        free(decodedEncoding);
        return 0;
}
