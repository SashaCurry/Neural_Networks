#include "iostream"
#include "cmath"
#include "vector"
#include "string"
#include "set"
#include "fstream"

using namespace std;

class Edge {
public:
	int from = -1;
	short rawFrom = 0;
	int to = -1;
	short rawTo = 0;
	short order = -1;
	Edge(int from, short rawFrom, int to = -1, short rawTo = 0, short order = -1) {
		this->from = from;
		this->rawFrom = rawFrom;
		this->to = to;
		this->rawTo = rawTo;
		this->order = order;
	}
	~Edge() {//It's destructor
	}

	bool operator == (Edge& edge) const {
		return (this->from == edge.from && this->to == edge.to);
	}
	bool operator < (Edge& edge) const {
		if (this->from == edge.from)
			return this->to < edge.to;
		return this->from < edge.from;
	}
	bool operator > (Edge& edge) const {
		if (this->from == edge.from)
			return this->to > edge.to;
		return this->from > edge.from;
	}
};



class Graph {
protected:
	string inFile;
	string outFile;
	set <int> vertexes;
	vector <Edge> edges;
	vector <string> errors;
	vector <pair <int, set <pair <int, int>>>> numEdges;

	bool checkStrDigit(string str, short raw) {
		if (str.empty()) {
			this->errors.push_back(string{ "Ошибка в строке " + to_string(raw) + ": некорретные данные (введены символы вместо цифр)" });
			return false;
		}
		for (int i = 0; i < str.length(); i++)
			if (!isdigit(str[i])) {
				this->errors.push_back(string{ "Ошибка в строке " + to_string(raw) + ": некорретные данные (введены символы вместо цифр)" });
				return false;
			}
		return true;
	}


	void checkSameEdge(int from, int to, short raw) {
		for (short i = 0; i < edges.size(); i++)
			if (edges[i].from == from && edges[i].to == to)
				this->errors.push_back(string{ "Ошибка в строке " + to_string(raw) + ": повторяющаяся дуга " +
											   to_string(from) + "->" + to_string(to) });
	}


	void checkSameNumEdge(int to, int order, short raw) {
		for (short i = 0; i < edges.size(); i++)
			if (edges[i].to == to && edges[i].order == order)
				this->errors.push_back(string{ "Ошибка в строке " + to_string(raw) + ": дуга с номером " + to_string(order) +
										       " в вершину " + to_string(to) + " уже существует" });
	}


	void checkNumVertex() {
		int vertexSize = vertexes.size();
		for (short i = 0; i < edges.size(); i++) {
			if (edges[i].from > vertexSize)
				this->errors.push_back(string{ "Ошибка в строке " + to_string(edges[i].rawFrom) + ": неправильная нумерация вершин." +
										       " Номер вершинa " + to_string(edges[i].from) + " больше количества вершин" });
			if (edges[i].to > vertexSize)
				this->errors.push_back(string{ "Ошибка в строке " + to_string(edges[i].rawTo) + ": неправильная нумерация вершин." +
										       " Номер вершинa " + to_string(edges[i].to) + " больше количества вершин" });
		}
	}


	void addNumEdges(int to, short order, int raw) {
		bool emptyVertex = true;
		for (short i = 0; i < numEdges.size(); i++)
			if (numEdges[i].first == to) {
				this->numEdges[i].second.insert(make_pair(order, raw));
				emptyVertex = false;
				break;
			}
		if (emptyVertex) {
			this->numEdges.push_back(make_pair(to, set <pair <int, int>> {make_pair(order, raw)}));
		}
	}


	void checkNumEdges() {
		for (int i = 0; i < numEdges.size(); i++) {
			int numPrev = 0;
			for (auto j = numEdges[i].second.begin(); j != numEdges[i].second.end(); j++) {
				pair <int, int> num = *j;
				if (num.first - numPrev == 1 || num.first - numPrev == 0)
					numPrev = num.first;
				else {
					this->errors.push_back(string{ "Ошибка в строке " + to_string(num.second) + ": неправильно заданы номера дуг" });
					numPrev = num.first;
				}
			}
		}
	}


	void getData() {
		ifstream fin(inFile);
		if (!fin.is_open())
			throw string{ "Файл " + inFile + " не найден! \n" };

		string str = "";
		string partEdge = "";
		short raw = 1;
		for (char el, elPrev = ' '; fin.get(el); elPrev = el) {
			if (el == '\n')
				raw++;
			else if (el == ' ' && elPrev == ',')
				continue;
			else if (el == '(' && partEdge == "") {
				partEdge = "from";
				str = "";
			}
			else if (el == ')' && !edges.empty() && edges.back().order != -1)
				this->errors.push_back(string{ "Ошибка в строке " + to_string(raw) + ": неправильно задана компонента. Формат: (a, b, n)" });
			else if (partEdge != "" && (el == ',' || el == ')')) {
				if (!checkStrDigit(str, raw))
					str = "-1";

				if (partEdge == "from") {
					if (str == "0")
						this->errors.push_back(string{ "Ошибка в строке " + to_string(raw) + ": вершины 0 быть не может" });

					this->vertexes.insert(stoi(str));
					this->edges.push_back(Edge(stoi(str), raw));
					partEdge = "to";
					str = "";
				}
				else if (partEdge == "to") {
					if (str == "0")
						this->errors.push_back(string{ "Ошибка в строке " + to_string(raw) + ": вершины 0 быть не может" });
					checkSameEdge(edges.back().from, stoi(str), raw);

					this->vertexes.insert(stoi(str));
					this->edges.back().to = stoi(str);
					this->edges.back().rawTo = raw;
					partEdge = "order";
					str = "";
				}
				else if (partEdge == "order") {
					checkSameNumEdge(edges.back().to, stoi(str), raw);

					this->edges.back().order = stoi(str);
					partEdge = "";
					str = "";

					addNumEdges(edges.back().to, edges.back().order, raw);
				}
			}
			else
				str += el;
		}

		checkNumVertex();
		checkNumEdges();
		fin.close();
	}
public:
	Graph(string inFile, string outFile) {
		this->inFile = inFile;
		this->outFile = outFile;
		getData();
	}
	~Graph() {//It's destructor
	}


	void printGraph() {
		if (edges.empty())
			cout << "\nФайл не содержит граф!";
		else {
			cout << "\nВершины: ";
			for (auto i = vertexes.begin(); i != vertexes.end(); i++)
				cout << *i << " ";
			cout << "\nДуги: ";
			for (short i = 0; i < edges.size(); i++)
				cout << "(" << edges[i].from << ", " << edges[i].to << ", " << edges[i].order << ") ";
		}
	}


	void printGraphInFile() {
		ofstream fout(outFile);
		if (!fout.is_open())
			throw string{ "Файл " + outFile + " не найден! \n" };

		if (!errors.empty())
			for (short i = 0; i < errors.size(); i++)
				cout << endl << errors[i];
		else {
			fout << "<graph>";
			for (auto i = vertexes.begin(); i != vertexes.end(); i++)
				fout << "\n\t<vertex>v" << *i << "</vertex>";
			for (short i = 0; i < edges.size(); i++) {
				fout << "\n\t<arc>";
				fout << "\n\t\t<from>v" << edges[i].from << "</from>";
				fout << "\n\t\t<to>v" << edges[i].to << "</to>";
				fout << "\n\t\t<order>" << edges[i].order << "</order>";
				fout << "\n\t</arc>";
			}
			fout << "\n</graph>";
			cout << "\nРезультат сохранён в файл " << outFile;	
		}
		fout.close();
	}
};



class GraphFunction : Graph {
protected:
	vector <Edge> sortedEdges;

	void sortEdges(vector <Edge> edges) {//edges - копия переменной this->edges
		while (!edges.empty()) {
			pair <Edge, short> minEdge = make_pair(edges[0], 0);
			for (short i = 1; i < edges.size(); i++)
				if (edges[i] < minEdge.first)
					minEdge = make_pair(edges[i], i);
			sortedEdges.push_back(minEdge.first);
			edges.erase(edges.begin() + minEdge.second);
		}
	}

	void checkCycles() {
		
	}
public:
	GraphFunction(string inFile, string outFile) : Graph(inFile, outFile) {
		sortEdges(this->edges);	 
	}
	~GraphFunction() {//It's destructor
	}

};



int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "ru");

	if (argc != 2 && argc != 3) {
		cout << "\nНеверно заданы параметры функции! \n";
		return 0;
	}

	string inFile = argv[1];
	string outFile = "out.txt";
	if (argc == 3)
		outFile = argv[2];

	try {
		GraphFunction(inFile, outFile);
	}
	catch (string& error) {
		cout << error;
	}
	cout << endl;
	return 0;
}
