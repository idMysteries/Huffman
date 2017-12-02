#include "Node.h"

using namespace std;
Node::Node(Node *L, Node *R) {
    left = L;
    right = R;
    freq = L->freq + R->freq;
    writeble = false;
}

Node::Node(pair<unsigned char, unsigned int> p, bool _writeble) {
    byte = p.first;
    freq = p.second;
    writeble = _writeble;
}