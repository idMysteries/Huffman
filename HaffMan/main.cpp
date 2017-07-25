#include <iostream>
#include <map>
#include <list>
#include <vector>
#include <string>
#include <fstream>
#include <locale>

using namespace std;

class Node {
public:
	int a;
	char c=0;
	Node *left = 0, *right = 0;
	Node() {};

	Node(Node *L, Node *R) {
		left = L;
		right = R;
		a = L->a + R->a;
	}
};

void BuildTable(Node *root, vector<bool> &code, map<char, vector<bool>> &table) 
{
	if (root->left) {
		code.push_back(0);
		BuildTable(root->left, code, table);
	}

	if (root->right) {
		code.push_back(1);
		BuildTable(root->right, code, table);
	}

	if (root->c)
		table[root->c] = code;
	if (code.size()) code.pop_back();
}

inline string getFileType(string file) {
	int id = file.find_last_of('.');
	if (id != string::npos)
		return file.substr(id + 1);
	else return "";
}

int main(int argc, char *argv[]) {
	setlocale(LC_ALL, "ru-RU");

	if (argc != 3) {
		cout << "Не верно введены параметры запуска" << endl;
		system("pause");
		return 1337; // leet error
	}
	else {
		if (argv[2] == "1") { // compressor
			ifstream in(argv[0], ios::out | ios::binary);
			map<char, int> m; 			  

			string fileType = getFileType(argv[0]);
			
			while (!in.eof()) {
				char c;
				c = in.get();
				m[c]++;
			}

			list<Node*> t;
			/*
				Почему лист?
				Этот контейнер позволяет легко вставлять элементы в произвольные места
				и быстро встраивать элементы в разные места

				Мы используем его для быстрого взятия и удаления первого элемента. 
				А так же для вставки в конец.

				По своей реализации лист - это список, а не массив
			*/

			for (auto i : m) {
				Node *p = new Node;
				p->c = i.first;
				p->a = i.second;
				t.push_back(p);
			}

			while (t.size() != 1) {
				t.sort([](const Node *l, const Node *r) -> bool {
					return l->a < r->a; // сортировка по неубыванию
				});

				Node *SonL = t.front();
				t.pop_front();
				Node *SonR = t.front();
				t.pop_front();

				/*
					Убираем мелкие части, добавляем крупные.
					В итоге останется только одна, крупная часть дерева
				*/

				t.push_back(new Node(SonL, SonR));
			}

			Node *root = t.front();

			vector<bool> code;
			
			map<char, vector<bool> > table;
			BuildTable(root, code, table);

			in.clear();
			in.seekg(0);

			ofstream out(argv[1]);
			int count = 0; char byte = 0;

			while (!in.eof()) {
				char c;
				c = in.get();

				vector<bool> x = table[c];

				for (auto i : x) {
					byte |= i << (7 - count);
					count++;
					if (count == 8) {
						count = 0;
						out << byte;
						byte = 0;
					}
				}
			}
			out.close();
			in.close();
		}
		else if (argv[2] == "2") { // decompressor

		}
		else { // ERROR (not leet)

		}
	}

	/*
		ЭТО ДЕКОМПРЕССИЯ, ЕЕ НАДО ДОДЕЛАТЬ ПОД ТЗ
		JUST DO IT!!!!

		КОД ГОТОВ НА ~69%~

		TODO:
			Тип входного файла
			Количество узлов в дереве
			Количество бит в последней байте
			Дерево в прямом порядке обхода
	*/

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
	return 0;
}