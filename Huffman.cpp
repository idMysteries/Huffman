#include "Huffman.h"
using namespace std;

inline pair<string, string> getFileNameAndType(string fileName) {
	auto dotIndex = fileName.find_last_of('.');
	if (dotIndex != string::npos) {
		return make_pair(fileName.substr(0, dotIndex), fileName.substr(dotIndex + 1));
	}
	return make_pair(fileName, "");
}

void Huffman::buildHuffmanTable(Node *root, vector<bool> &code) {
	if (root->left) {
		code.push_back(0);
		buildHuffmanTable(root->left, code);
	}
	if (root->right) {
		code.push_back(1);
		buildHuffmanTable(root->right, code);
	}
	if (root->writeble) {
		huffmanTable[root->byte] = code;
	}
	if (code.size()) {
		code.pop_back();
	}
}

void Huffman::writeHuffmanTreeToFile(Node *root, ofstream &out) {
	out.write((char *)&root->byte, 1);
	out.write((char *)&root->writeble, 1);
	out.write((char *)&root->freq, 4);
	if (root->left) {
		out << 'L';
		writeHuffmanTreeToFile(root->left, out);
	}
	if (root->right) {
		out << 'R';
		writeHuffmanTreeToFile(root->right, out);
	}
}

inline void Huffman::readHuffmanTreeFromFile(ifstream &in) {
	head = new Node();
	unsigned char to;
	in.read((char *)&to, 1);
	if (to == 'H') {
		in.read((char *)&head->byte, 1);
		in.read((char *)&head->writeble, 1);
		in.read((char *)&head->freq, 4);
		--treeNodesCount;
		readHuffmanTreeNodesFromFile(in, head);
	}
}

inline void Huffman::readHuffmanTreeNodesFromFile(ifstream &in, Node *root) {
	unsigned int freq;
	unsigned char to, byte, writeble;
	while (treeNodesCount-- != 0) {
		in.read((char *)&to, 1);
		in.read((char *)&byte, 1);
		in.read((char *)&writeble, 1);
		in.read((char *)&freq, 4);
		if (to == 'L') {
			root->left = new Node(make_pair(byte, freq), writeble);
			root->left->prev = root;
			root = root->left;
		} else if (to == 'R') {
			while (root->prev->right != nullptr) {
				root = root->prev;
			}
			root->prev->right = new Node(make_pair(byte, freq), writeble);
			root->prev->right->prev = root->prev;
			root = root->prev->right;
		}
	}
}

inline void Huffman::countFreq(map<unsigned char, unsigned int> &m) {
	ifstream in(argv[1], ios::out | ios::binary);
	if (!in) {
		cout << "File not found";
		exit(1);
	}

	unsigned char c;
	while (in.peek() != EOF) {
		c = in.get();
		m[c]++;
	}
	in.close();
}

inline unsigned char Huffman::buildCompressedData(map<unsigned char, vector<bool>> &huffmanTable) {
	ifstream in(argv[1], ios::out | ios::binary);
	if (!in) {
		cout << "File not found";
		exit(1);
	}

	unsigned char count = 0, c, byte = 0;
	while (in.peek() != EOF) {
		c = in.get();
		for (auto i : huffmanTable[c])	{
			byte |= i << 7 - count;
			if (++count == 8) {
				count = 0;
				compressedBytes.push_back(byte);
				byte = 0;
			}
		}
	}
	if (count != 0) {
		compressedBytes.push_back(byte);
	} else {
		count = 8;
	}
	in.close();
	return count;
}

inline void Huffman::writeCompressedData(map<unsigned char, vector<bool>> &huffmanTable) {
	auto decompressedFileType = getFileNameAndType(argv[1]);
	ofstream out(string(argv[2]) + TYPE, ios::binary | ios::out);
	out.write(decompressedFileType.second.c_str(), decompressedFileType.second.size() + 1);
	unsigned char bitsCount = buildCompressedData(huffmanTable);
	out.write((char *)&treeNodesCount, 4);
	out.write((char *)&bitsCount, 1);
	out << 'H';
	writeHuffmanTreeToFile(head, out);
	out.write((char *)&compressedBytes[0], compressedBytes.size());
	out.close();
}

void Huffman::compress() {
	map<unsigned char, unsigned int> m;
	countFreq(m);  
	list<Node *> t;
	for (auto &i : m) {
		t.push_back(new Node(i, true));
	}
	m.clear();
	treeNodesCount = t.size();
	while (t.size() > 1) {
		t.sort([](const Node *l, const Node *r) -> bool {
			return l->freq < r->freq;
		});
		Node *SonL = t.front();
		t.pop_front();
		Node *SonR = t.front();
		t.pop_front();
		t.push_back(new Node(SonL, SonR));
		treeNodesCount++;
	}
	head = t.front();
	vector<bool> code;
	buildHuffmanTable(head, code);
	code.clear();
	writeCompressedData(huffmanTable);
}

void Huffman::decompress() {
	ifstream in(string(argv[2]) + TYPE, ios::in | ios::binary);
	if (!in) {
		cout << "File not found";
		exit(1);
	}

	mkdir(getFileNameAndType(argv[2]).first.c_str());
	string decompressedFileType;
	unsigned char tempChar, byte;
	do {
		in.read((char *)&tempChar, 1);
		decompressedFileType += tempChar;
	} while (tempChar != '\0');
	ofstream out(string(argv[2]) + "\\" + string(argv[2]) + "." + decompressedFileType, 
						ios::out | ios::binary);
	if (!out) {
		cout << "File not created";
		exit(1);
	}
	unsigned char lastBitsCount;
	in.read((char *)&treeNodesCount, 4);
	in.read((char *)&lastBitsCount, 1);
	readHuffmanTreeFromFile(in);
	Node *p = head;
	unsigned int count = 0;
	in.read((char *)&byte, 1);
	while (!in.eof() || count < lastBitsCount) {
		bool b = byte & (1 << (7 - count));
		if (b) {
			p = p->right;
		} else {
			p = p->left;
		}
		if (p->writeble) {
			out.write((char *)&p->byte, 1);
			p = head;
		}
		bool isEnd = in.peek() == EOF;
		if (++count == (isEnd ? lastBitsCount : 8)) {
			if (isEnd)
				break;
			count = 0;
			in.read((char *)&byte, 1);
		}
	}
	in.close();
	out.close();
}

Huffman::Huffman(char *_argv[]) {
	argv = _argv;
}

void Huffman::deleteHuffmanTree(Node *root) {
	if (root->left) {
		deleteHuffmanTree(root->left);
	}
	if (root->right) {
		deleteHuffmanTree(root->right);
	}
	delete root;
}

Huffman::~Huffman() {
	huffmanTable.clear();
	compressedBytes.clear();
	deleteHuffmanTree(head);
}