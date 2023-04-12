// reading a text file
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
using namespace std;

int main(int argc, char *argv[]) {
    // printf("%d",argc);
    // printf("%s", argv[1]);
    std::ifstream inFile;
    inFile.open(argv[1]); //open the input file

    std::stringstream strStream;
    strStream << inFile.rdbuf(); //read the file
    std::string str = strStream.str(); //str holds the content of the file

    printf("input length: %d\n", (int)str.length());

    return 1;
}