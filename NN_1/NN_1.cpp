#include "iostream"
#include "cmath"
#include "vector"
#include "string"
#include "map"
#include "set"
#include "fstream"

using namespace std;

class Edge {
public:
	short from;
	short to;
	short order;
	Edge() {
		this->from = 0;
		this->to = 0;
		this->order = 0;
	}
	Edge(short from, short to = 0, short order = 0) {
		this->from = from;
		this->to = to;
		this->order = order;
	}
};


class Graph {
private:
	string inFile;
	string outFile;
	set <short> vertexes;
	vector <Edge> edges;

	bool isStrDigit(string str) {
		for (int i = 0; i < length(str); i++)
			if (!isdigit(str[i]))
				return false;
		return true;
	}

	bool isHaveSymbols(string str) {
		for (int i = 0; i < length(str); i++)
		   if (isalpha((unsigned char)str[i]))
				return true;
 		return false;
	}		
public:
	Graph() {
		this->inFile = "";
		this->outFile = "";
	}	
	Graph(string inFile, outFile) {
		this->inFile = inFile;
		this->outFile = outFile;
	}
	~Graph() {//It's destructor
	}
	
	void getData() {
		ifstream fin(inFile);
		if (!fin.is_open())
			throw string{"Файл " + inFile + " не найден! \n"};
		
		string str = "";
		char el;
		string partEdge = "";
		short raw = 1;	
		while (fin.get(el)) {
			if (el == '\n')
				raw++;
			else if (el == ' ' || el == '\t')
				continue;
			else if (el == '(' && partEdge == "")
				partEdge = "from";
			else if (el == ',' || el == ')') {
				if (!isStrDigit(str)) {
					if (isHaveSymbols(str))
						throw string{"Ошибка в строке " + raw + ": некорректные данные (введены буквы вместо цифр)"};
					else
						throw string{"Ошибка в строке " + raw + ": неправильно задана компонента. Формат: (a, b, n)"};
				}

				if (partEdge == "from") {
					vertexes.include(stoi(str));
					edges.push_back(Edge(stoi(str)));
					partEdge = "to";
				}
				else if (partEdge == "to") {
					vertexes.include(stoi(str));
					edges.back().to = stoi(str);
					partEdge = "order";
				}
				else if (partEdge == "order") {
					edges.back().order = stoi(str);
					partEdge = "";
				}
			}	
			else
				str += el;
		}
	}	
;


int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "ru");
	
	if (argc != 2 && argc != 3) {
		cout << "\nНеверно заданы параметры функции!";
		return 0;
	}
	
   	string inFile = argv[1];
	string outFile = argv[2];
	createGraph(inFile, outFile);
	cout << endl;
	return 0;
}
