#include <iostream>
#include <string>
#include <locale>

#include "Huffman.h"

using namespace std;

int main(int argc, char *argv[]) {
	setlocale(LC_ALL, "");
	if (argc != 4) {
		cout << "Bad arguments" << endl;
		return 1;
	}
	Huffman huffman(argv);
	if (string(argv[3]) == "1") {
		huffman.compress();
	} else if (string(argv[3]) == "2") {
		huffman.decompress();
	} else {
		cout << "Bad compression type" << endl;
		return 2;
	}
	return 0;
}