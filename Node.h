#ifndef node_h
#define node_h

#include <iostream>

using namespace std;

class Node {
public:
	unsigned int freq = 0;
	unsigned char byte = 0;
	bool writeble;
	Node *prev = nullptr;
	Node *left = nullptr, *right = nullptr;
	Node() {}

	Node(Node *L, Node *R);
	Node(pair<unsigned char, unsigned int> p, bool _writeble);
};
#endif