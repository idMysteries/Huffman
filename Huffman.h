#ifndef huffman_h
#define huffman_h

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <list>
#include <io.h>

#include "Node.h"

using namespace std;
inline pair<string, string> getFileNamePair(string fileName);

class Huffman {
private:
	const string TYPE = ".tar";
	char **argv;
	Node *head;
	map<unsigned char, vector<bool>> huffmanTable;
	vector<unsigned char> compressedBytes;
	int treeNodesCount = 0;
	void buildHuffmanTable(Node *root, vector<bool> &code);
	void deleteHuffmanTree(Node *root);
	void writeHuffmanTreeToFile(Node *root, ofstream &out);
	inline void readHuffmanTreeFromFile(ifstream &in);
	inline void readHuffmanTreeNodesFromFile(ifstream &in, Node *root);
	inline void countFreq(map<unsigned char, unsigned int> &m);
	inline unsigned char buildCompressedData(map<unsigned char, vector<bool>> &huffmanTable);
	inline void writeCompressedData(map<unsigned char, vector<bool>> &huffmanTable);
public:
	void compress();
	void decompress();
	Huffman(char *_argv[]);
	~Huffman();
};
#endif