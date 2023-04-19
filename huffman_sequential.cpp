#include <iostream>
#include <cstdlib>
struct node {
 
    // One of the input characters
    char data;
 
    // Frequency of the character
    unsigned freq;
    // Left and right child of this node
    struct node *left;
    struct node *right;
};
struct tree {
     
    unsigned size;
    unsigned capacity;
    struct node **array;
};
struct tree *newNode(char data, unsigned frequency) {
    struct node *result = (struct node*)malloc(sizeof(struct node));
    result->left = NULL;
    result->right = NULL;
    result->data = data;
    result->freq = frequency;
    return result;
}
struct tree *newTree(unsigned capacity) {
    struct tree *result = (struct tree*)malloc(sizeof(struct tree));
    result->size = 0;
    result->capacity = capacity;
    result->array = (struct node**)malloc(result->capacity * sizeof(struct node*));
    return result
}
void swapNode(struct node** first, struct node** second) {
    struct node* temp = *first;
    *first = *second;
    *second = temp;
}
void insertNode(struct tree* bTree, struct node* bNode) {
    int index = bTree->size - 1;
    while (index && bNode->freq < bTree->array[(index - 1)/2]->freq) {
        index = (index - 1) / 2;
    } 
    bTree->array[index] = bNode;
    bTree->size = bTree->size + 1;
}
void buildHtree(struct tree* bTree, int index) {
    int least = index;
    int left = 2 * index + 1;
    int right = left + 1;
    bool cond1 = left < bTree->size;
    bool cond2 = bTree->array[left]->freq < bTree->array[least]->freq;
    if (cond1 && cond2) {
        least = left;
    }
    bool cond3 = right < bTree->size;
    bool cond4 = bTree->array[right]->freq < bTree->array[least]->freq;
    if (cond3 && cond4) {
        least = right;
    }
    if (least != index) {
        swapNode(&bTree->array[least], &bTree->array[index]);
        buildHtree(bTree, least);
    }
}
struct node* getMin(struct tree* bTree) {
    int last = bTree->size - 1;
    struct node* result = bTree->array[0];
    bTree->array[0] = last;
    bTree->size = bTree->size - 1;
    createHtree(bTree, 0);
    return result;
}
void createHTree(struct tree* bTree) {
    int max = bTree->size - 1;
    for (int i = (max - 1)/2; i >= 0; i--) {
        buildHtree(bTree, i);
    }
}

bool isLeaf (struct node* bNode) {
    return !(bNode->left) && !(bNode->right);
}

struct tree* heapCreate(char input[], int freq[], int size) {
    struct tree* result = newTree(size);
    result->size = size;
    for (int i = 0; i < size; i++) {
        result->array[i] = newNode(input[i], freq[i]);
    }
    createHtree(result);
    return result;
}

struct node* createHuffmanTree(char input[], int freq[], int size) {
    struct node *left, *right, *root;
    struct tree* heap = heapCreate(input, freq, size);
    while (heap->size != 1) {
        left = getMin(heap);
        right = getMin(heap);
        //'+' is for internal nodes that have the frequency of the sum of its 2 children
        root = newNode('+', left->freq + right->freq);
        root->left = left;
        root->right = right;
        insertNode(heap, root);
    }
    return getMin(heap);
}