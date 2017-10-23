#include <iostream>
#include <map>
#include <list>
#include <vector>
#include <string>
#include <fstream>
#include <locale>
#include <io.h>

#define TYPE ".tar"
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

	explicit Node(pair<char, unsigned int> p) {
		byte = p.first;
		freq = p.second;
	}
};

inline pair<string, string> getFileNamePair(string fileName) {
	auto id = fileName.find_last_of('.');
	if (id != string::npos)
		return make_pair(fileName.substr(0, id), fileName.substr(id + 1));
	return make_pair(fileName, "");
}

class Huffman {
private:
	char **argv, to, byte = 0;
	Node *head;
	map<char, vector<bool>> table;
	vector<char> compressedBytes;
	int nodesCount = 0;

	char c;
	
	void buildTable(Node *root, vector<bool> &code) {
		if (root->left) {
			code.push_back(0);
			buildTable(root->left, code);
		}
		
		if (root->right) {
			code.push_back(1);
			buildTable(root->right, code);
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

	void createTreeFromFile(ifstream &in) {
		head = new Node();
		in.read((char*)&to, 1);
		if (to == 'H'){
			in.read((char*)&head->byte, 1);
			in.read((char*)&head->freq, 4);
			--nodesCount;
			readTreeNodes(in, head);
		}
	}
	
	void readTreeNodes(ifstream &in, Node *root) {
		unsigned int freq;
		while (nodesCount-- != 0) { 	
			in.read((char*)&to, 1);
			in.read((char*)&byte, 1);
			in.read((char*)&freq, 4);
		
			if (to == 'L') {
				root->left = new Node(make_pair(byte, freq));
				root->left->prev = root;
				root = root->left;
			} 
			else if (to == 'R') {
				while (root->prev->right != nullptr) {
					root = root->prev;
				}
				root->prev->right = new Node(make_pair(byte, freq));
				root->prev->right->prev = root->prev;
				root = root->prev->right;
			}
		}
	}

	void freqCounting(map<char, unsigned int> &m) {
		ifstream in(argv[1], ios::out | ios::binary);
		while ((c = in.get()) != -1) {
			m[c]++;
		}
		in.close();
	}	

	char createCompressedData(map<char, vector<bool>> &table) {
		ifstream in(argv[1], ios::out | ios::binary);
		char count = 0;			
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
		in.close();
		
		return count;
	}
	
	void writeCompressedData(map<char, vector<bool>> &table) {
		auto fileType = getFileNamePair(argv[1]);
			
		ofstream out(string(argv[2]) + TYPE, ios::binary | ios::out);
		out.write(fileType.second.c_str(), fileType.second.size() + 1);
		
		char bitsCount = createCompressedData(table);
	
		out.write((char*)&nodesCount, 4);
		out.write((char*)&bitsCount, 1);
	
		out << 'H';
		writeTree(head, out);
	
		out.write((char*)&compressedBytes[0], compressedBytes.size());
		out.close();
	}

public:
	void compress(){
		map<char, unsigned int> m; 			  
		freqCounting(m);

		list<Node*> t;
		for (auto &i : m) {
			t.push_back(new Node(i));
		}
		m.clear();
		nodesCount = t.size();
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
		head = t.front();

		vector<bool> code;
		buildTable(head, code);
		code.clear();

		writeCompressedData(table);
	}

	void decompress(){
		ifstream in(string(argv[2]) + TYPE, ios::in | ios::binary);
		mkdir(argv[2]);
		
		string fileType;
		char b;
		do {
			in.read((char*)&b, 1);
			fileType += b;
		} while (b != '\0');
		ofstream out(string(argv[2]) + "\\" + string(argv[2]) + "." + fileType, ios::out | ios::binary);
		
		char lastBitsCount;
		in.read((char*)&nodesCount, 4);
		in.read((char*)&lastBitsCount, 1);
		createTreeFromFile(in);
		Node *p = head;

		unsigned int count = 0;
		in.read((char *)&byte, 1);
		while (!in.eof() || count < lastBitsCount) {
			bool b = byte & (1 << (7 - count));
			if (b) p = p->right;
			else p = p->left;
			
			if (p->byte) {
				out << p->byte;
				p = head;
			}
	
			bool isEnd = in.peek() == EOF;
			if (++count == (isEnd ? lastBitsCount : 8)) {
				if (isEnd) break;
				count = 0;
				in.read((char *)&byte, 1);
			}
		}
		in.close();
		out.close();
	}

	Huffman(char *_argv[]){
		argv = _argv;
	}
};

int main(int argc, char *argv[]) {
	setlocale(LC_ALL, "ru-RU");
	if (argc != 4) {
		cout << "Bad arguments" << endl;
		system("pause");
		return 1337;
	}

	Huffman huffman(argv);

	if (string(argv[3]) == "1") {
		huffman.compress();
	}
	else if (string(argv[3]) == "2") {
		huffman.decompress();
	}
	else {
		cout << "Bad compression type" << endl;
		system("pause");
		return 1337;
	}
	system("pause");
	return 0;
}