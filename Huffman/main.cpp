#include <iostream>
#include <map>
#include <list>
#include <vector>
#include <string>
#include <fstream>
#include <locale>
#include <iomanip>

using namespace std;

class Node {
public:
	int a = 0;
	char c = 0;
	Node *left = 0, *right = 0;
	Node() {}

	Node(Node *L, Node *R) {
		left = L;
		right = R;
		a = L->a + R->a;
	}

	explicit Node(pair<char, int> p) {
		c = p.first;
		a = p.second;
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

	if (root->c)
		table[root->c] = code;
	if (code.size()) code.pop_back();
}

void writeTree(Node *root, ofstream &out, int k = 0) {
	out << root->c;
	out.write((char*)&root->a, sizeof root->a);

	cout << root->c << ':' << root->a << ' ' << k << endl;

	if (root->left) {
		cout  << 'L';
		writeTree(root->left, out, k + 1);
	}

	if (root->right) {
		cout << 'R';
		writeTree(root->right, out, k + 1);
	}
}

inline string getFileType(string file) {
	auto id = file.find_last_of('.'); // example.exe.txt.zip
	if (id != string::npos)
		return file.substr(id + 1);		// zip
	return "";
}

int main(int argc, char *argv[]) {
	setlocale(LC_ALL, "ru-RU");
	if (argc != 4) {
		cout << "Bad arguments" << endl;
		system("pause");
		return 1337;
	}
	if (string(argv[3]) == "1") {
		cout << "Compression started" << endl;
		ifstream in(argv[1], ios::out | ios::binary);
		map<char, int> m; 			  
		string fileType = getFileType(argv[1]);
			
		while (!in.eof()) {
			char c = in.get();
			m[c]++;
		}

		list<Node*> t;
		for (auto &i : m) {
			t.push_back(new Node(i));
		}

		while (t.size() != 1) {
			t.sort([](const Node *l, const Node *r) -> bool {
			       return l->a < r->a;
		       });
			Node *SonL = t.front();
			t.pop_front();
			Node *SonR = t.front();
			t.pop_front();
			t.push_back(new Node(SonL, SonR));
		}

		Node *root = t.front();

		vector<bool> code;
		map<char, vector<bool>> table;
		buildTable(root, code, table);

		in.clear();
		in.seekg(0);

		ofstream out(argv[2], ios::binary | ios::out);
		out.write(reinterpret_cast<char*>(&fileType), fileType.size() + 1);
		auto count = 0;
		char byte = 0;
						
		vector<char> bytes;

		while (!in.eof()) {
			char c = in.get();
			auto x = table[c];

			for (auto &i : x) {
				byte |= i << 7 - count;
				if (++count == 8) {
					count = 0;
					bytes.push_back(byte);
					byte = 0;
				}
			}
		}

		char lastByte = bytes[bytes.size() - 1];

		char lastBits = 8;

		for (int i = 0; i < 8; ++i) {
			if ((lastByte << i) & 1) {
				lastBits -= i;
				break;
			}
		}

		int nodesCount = bytes.size();

		out.write((char*)&nodesCount, sizeof nodesCount);
		out.write((char*)&lastBits, sizeof lastBits);

		writeTree(root, out);

		out.write((char*)&bytes, bytes.size());

		cout << bytes.size() << endl;

		out.close();
		in.close();
		cout << "Compression is finished" << endl;
	}
	else if (string(argv[3]) == "2") {
		/*
			ifstream compressedIn("compressed.bin", ios::in | ios::binary);
			ofstream decompressedOut("decompressed.txt");

			Node *p = root;
			count = 0;
			compressedIn >> byte;

			while (!compressedIn.eof()) {
			bool b = byte & (1 << (7 - count));
			if (b) p = p->right;
			else p = p->left;
			if (p->c) {
			decompressedOut << p->c;
			p = root;
			}
			count++;
			if (count == 8) {
			count = 0;
			compressedIn >> byte;
			}
			}

			compressedIn.close();
			decompressedOut.close();
			*/
	}
	else { 

	}
	system("pause");
	return 0;
}