#include <iostream>
#include <map>
#include <list>
#include <vector>
#include <string>
#include <fstream>
#include <locale>
#include <iomanip>
#include <io.h>

using namespace std;

class Node {
public:
	unsigned int freq = 0;
	char byte = 0;
	Node *prev = nullptr;
	Node *left = nullptr, *right = nullptr;
	Node() {}

	Node(Node *L, Node *R) {
		left = L;
		right = R;
		freq = L->freq + R->freq;
	}

	explicit Node(pair<char, int> p) {
		byte = p.first;
		freq = p.second;
	}
};

void buildTable(Node *root, vector<bool> &code, map<char, vector<bool>> &table) {
	if (root->left) {
		code.push_back(0);
		buildTable(root->left, code, table);
	}
	
	if (root->right) {
		code.push_back(1);
		buildTable(root->right, code, table);
	}

	if (root->byte)
		table[root->byte] = code;
	if (code.size()) 
		code.pop_back();
}

void writeTree(Node *root, ofstream &out) {
	out.write((char*)&root->byte, 1); 							
	out.write((char*)&root->freq, 4); 

	if (root->left) {
		out << 'L';
		writeTree(root->left, out);
	}

	if (root->right) {
		out << 'R';
		writeTree(root->right, out);
	}
}

inline pair<string, string> getFileNamePair(string file) {
	auto id = file.find_last_of('.');
	if (id != string::npos)
		return make_pair(file.substr(0, id), file.substr(id + 1));
	return make_pair("", "");
}

void readTreeNodes(ifstream &in, unsigned int &nodesCount, Node *node) {
	char to, byte;
	unsigned int freq;
	while (nodesCount-- != 0) { 	
		in.read((char*)&to, 1);
		in.read((char*)&byte, 1);
		in.read((char*)&freq, 4);
	
		if (to == 'L') {
			node->left = new Node(make_pair(byte, freq));
			node->left->prev = node;
			node = node->left;
		} 
		else if (to == 'R') {
			while (node->prev->right != nullptr) {
				node = node->prev;
			}
			node->prev->right = new Node(make_pair(byte, freq));
			node->prev->right->prev = node->prev;
			node = node->prev->right;
		}
	}
}

Node* createTreeFromFile(ifstream &in, unsigned int &nodesCount) {
	Node *root = new Node();
    char type;
	in.read((char*)&type, 1);
    if (type == 'H'){
		in.read((char*)&root->byte, 1);
		in.read((char*)&root->freq, 4);
		readTreeNodes(in, --nodesCount, root);
	}
	return root;
}

int main(int argc, char *argv[]) {
	setlocale(LC_ALL, "ru-RU");
	if (argc != 4) {
		cout << "Bad arguments" << endl;
		system("pause");
		return 1337;
	}
	if (string(argv[3]) == "1") {
		ifstream in(argv[1], ios::out | ios::binary);
		
		map<char, unsigned int> m; 			  
		auto fileType = getFileNamePair(argv[1]);
		
		char c;
		while ((c = in.get()) != -1) {
			m[c]++;
		}
		in.clear();
		in.seekg(0);
		
		list<Node*> t;
		for (auto &i : m) {
			t.push_back(new Node(i));
		}
		m.clear();
		unsigned int nodesCount = t.size();
		
		while (t.size() != 1) {
			t.sort([](const Node *l, const Node *r) -> bool {
			       return l->freq < r->freq;
		       });
			Node *SonL = t.front();
			t.pop_front();
			Node *SonR = t.front();
			t.pop_front();
			t.push_back(new Node(SonL, SonR));
			nodesCount++;
		}
		Node *root = t.front();

		vector<bool> code;
		map<char, vector<bool>> table;

		buildTable(root, code, table);
		code.clear();

		ofstream out(argv[2], ios::binary | ios::out);
		out.write(fileType.second.c_str(), fileType.second.size() + 1);
		
		vector<char> compressedBytes;
		char count = 0;
		char byte = 0;				
		while ((c = in.get()) != -1) {
			for (auto i : table[c]) {
				byte |= i << 7 - count;
				if (++count == 8) {
					count = 0;
					compressedBytes.push_back(byte);
					byte = 0;
				}
			}
		}

		if (count != 0)
			compressedBytes.push_back(byte);
		else count = 8;

		out.write((char*)&nodesCount, 4);
		out.write((char*)&count, 1);

		out << 'H';
		writeTree(root, out);

		out.write((char*)&compressedBytes[0], compressedBytes.size());
		out.close();
		in.close();
	}
	else if (string(argv[3]) == "2") {
		ifstream in(argv[2], ios::in | ios::binary);
		auto decFileType = getFileNamePair(string(argv[1]));
		mkdir(decFileType.first.c_str());
		ofstream out(decFileType.first + "\\" + string(argv[1]), ios::out | ios::binary);

		string fileType;
		char b;
		do {
			in.read((char*)&b, 1);
			fileType += b;
		} while (b != '\0');

        unsigned int nodesCount;
        char lastByteSize;
        in.read((char*)&nodesCount, 4);
		in.read((char*)&lastByteSize, 1);

        Node *root = createTreeFromFile(in, nodesCount);
		Node *p = root;

		char byte = 0;
		unsigned int count = 0;
		in.read((char *)&byte, 1);
		while (!in.eof() || count < lastByteSize) {
			bool b = byte & (1 << (7 - count));
			if (b) p = p->right;
			else p = p->left;
			
			if (p->byte) {
				out << p->byte;
				p = root;
			}
	
			bool isEnd = in.peek() == EOF;
			if (++count == (isEnd ? lastByteSize : 8)) {
				if (isEnd) break;
				count = 0;
				in.read((char *)&byte, 1);
			}
		}
		in.close();
		out.close();
	}
	else { 
		cout << "Bad arguments" << endl;
		system("pause");
		return 1337;
	}
	return 0;
}