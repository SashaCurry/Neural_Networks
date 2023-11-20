#include "iostream"
#include "cmath"
#include "vector"
#include "string"
#include "set"
#include "map"
#include "fstream"
#include "algorithm"

using namespace std;

const double EXP = 2.71;

class File {
public:
	static string read(string inFile) {
		ifstream fin(inFile);
		if (!fin.is_open())
			throw string{ "Файл " + inFile + " не найден! \n" };

		string str = "";
		for (char el; fin.get(el);)
			str += el;

		fin.close();
		return str;
	}

	static void write(string str, string outFile) {
		ofstream fout(outFile);
		if (!fout.is_open())
			throw string{ "Недостаточно прав для создания файла " + outFile + "!\n" };

		fout << str;
		fout.close();
	}

	static void append(string str, string outFile) {
		ofstream fout(outFile, ios::app);
		if (!fout.is_open())
			throw string{ "Файл " + outFile + " не найден! \n" };

		fout << endl << str;
		fout.close();
	}
};



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

	Edge reverse() {
		swap(from, to);
		return *this;
	}
};



class Graph {
protected:
	string inFile;
	string outFile;
	set <int> vertexes;
	vector <Edge> edges;
	set <string> errors;
	vector <pair <int, set <pair <int, int>>>> numEdges;

	bool checkStrDigit(string str, short raw) {
		if (str.empty()) {
			this->errors.insert(string{ "Ошибка в строке " + to_string(raw) + ": некорретные данные (введены символы вместо цифр)" });
			return false;
		}
		for (int i = 0; i < str.length(); i++)
			if (!isdigit(str[i])) {
				this->errors.insert(string{ "Ошибка в строке " + to_string(raw) + ": некорретные данные (введены символы вместо цифр)" });
				return false;
			}
		return true;
	}


	void checkSameEdge(int from, int to, short raw) {
		for (short i = 0; i < edges.size(); i++)
			if (edges[i].from == from && edges[i].to == to)
				this->errors.insert(string{ "Ошибка в строке " + to_string(raw) + ": повторяющаяся дуга " +
											   to_string(from) + "->" + to_string(to) });
	}


	void checkSameNumEdge(int to, int order, short raw) {
		for (short i = 0; i < edges.size(); i++)
			if (edges[i].to == to && edges[i].order == order)
				this->errors.insert(string{ "Ошибка в строке " + to_string(raw) + ": дуга с номером " + to_string(order) +
											   " в вершину " + to_string(to) + " уже существует" });
	}


	void checkNumVertex() {
		int vertexSize = vertexes.size();
		for (short i = 0; i < edges.size(); i++) {
			if (edges[i].from > vertexSize)
				this->errors.insert(string{ "Ошибка в строке " + to_string(edges[i].rawFrom) + ": неправильная нумерация вершин." +
											   " Номер вершинa " + to_string(edges[i].from) + " больше количества вершин" });
			if (edges[i].to > vertexSize)
				this->errors.insert(string{ "Ошибка в строке " + to_string(edges[i].rawTo) + ": неправильная нумерация вершин." +
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
					this->errors.insert(string{ "Ошибка в строке " + to_string(num.second) + ": неправильно заданы номера дуг" });
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
				this->errors.insert(string{ "Ошибка в строке " + to_string(raw) + ": неправильно задана компонента. Формат: (a, b, n)" });
			else if (partEdge != "" && (el == ',' || el == ')')) {
				if (!checkStrDigit(str, raw))
					str = "-1";

				if (partEdge == "from") {
					if (str == "0")
						this->errors.insert(string{ "Ошибка в строке " + to_string(raw) + ": вершины 0 быть не может" });

					this->vertexes.insert(stoi(str));
					this->edges.push_back(Edge(stoi(str), raw));
					partEdge = "to";
					str = "";
				}
				else if (partEdge == "to") {
					if (str == "0")
						this->errors.insert(string{ "Ошибка в строке " + to_string(raw) + ": вершины 0 быть не может" });
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

		if (edges.empty())
			throw (string{ "\nФайл не содержит граф!" });
		else if (!errors.empty()) {
			for (auto i = errors.begin(); i != errors.end(); i++)
				cout << endl << *i;
			errors.clear();
		}
	}
	~Graph() {//It's destructor
	}


	void printGraph() {
		if (!errors.empty())
			return;
		cout << "\nВершины: ";
		for (auto i = vertexes.begin(); i != vertexes.end(); i++)
			cout << *i << " ";
		cout << "\nДуги: ";
		for (short i = 0; i < edges.size(); i++)
			cout << "(" << edges[i].from << ", " << edges[i].to << ", " << edges[i].order << ") ";
	}


	void printGraphInFile() {
		if (!errors.empty())
			return;

		ofstream fout(outFile);
		if (!fout.is_open())
			throw string{ "Файл " + outFile + " не найден! \n" };

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
		fout.close();
	}
};



class GraphFunction : protected Graph {
protected:
	string graphFunction = "";

	void checkCycles(int curV, int* usedV) {
		usedV[curV - 1] = 1;
		for (short i = 0; i < edges.size(); i++)
			if (edges[i].from == curV) {
				if (usedV[edges[i].to - 1] == 1)
					errors.insert(string{ "Ошибка в строке " + to_string(edges[i].rawTo) + ": в графе существует цикл" });
				else {
					checkCycles(edges[i].to, usedV);
					usedV[edges[i].to - 1] = 0;
				}
			}
	}


	vector <Edge> sortEdges(vector <Edge> edges) {//edges - копия переменной this->edges
		for (short i = 0; i < edges.size(); i++)
			edges[i].reverse();

		vector <Edge> res;
		while (!edges.empty()) {
			pair <Edge, short> minEdge = make_pair(edges[0], 0);
			for (short i = 1; i < edges.size(); i++)
				if (edges[i] < minEdge.first)
					minEdge = make_pair(edges[i], i);
			res.push_back(minEdge.first);
			edges.erase(edges.begin() + minEdge.second);
		}
		
		reverse(res.begin(), res.end());
		for (short i = 0; i < res.size() - 1; i++)
			if (res[i].from == res[i + 1].from && res[i].to > res[i + 1].to)
				swap(res[i], res[i + 1]);

		return res;
	}


	int findToAndDeleteEdge(vector <Edge>& edges, int vFrom) {
		for (short i = 0; i < edges.size(); i++)
			if (edges[i].from == vFrom) {
				int res = edges[i].to;
				edges.erase(edges.begin() + i);
				return res;
			}
		return 0;
	}

	string createGraphFunction(vector <Edge> edges, int v, int vPrev = 0, string acc = "") {
		int vNext = findToAndDeleteEdge(edges, v);
		if (vNext == 0) {
			if (findToAndDeleteEdge(edges, vPrev) == 0)
				return acc += to_string(v) + ")";
			else
				return acc += to_string(v) + ",";
		}

		acc += to_string(v) + "(";
		while (vNext != 0) {
			acc = createGraphFunction(edges, vNext, v, acc);
			vNext = findToAndDeleteEdge(edges, v);
		}

		if (vNext == 0) {
			if (findToAndDeleteEdge(edges, vPrev) == 0)
				acc += ")";
			else
				acc += ",";
		}
		return acc;
	}
public:
	GraphFunction(string inFile, string outFile = "out.txt") : Graph(inFile, outFile) {
		int* usedV = new int[*(--vertexes.end())]{};
		auto i = vertexes.find(0) != vertexes.end() ? ++vertexes.begin() : vertexes.begin();
		for (; i != vertexes.end(); i++)
			checkCycles(*i, usedV);
		delete[] usedV;

		if (!errors.empty()) {
			for (auto i = errors.begin(); i != errors.end(); i++)
				cout << endl << *i;
			return;
		}

		vector <Edge> sortedEdges = sortEdges(this->edges);
		this->graphFunction = createGraphFunction(sortedEdges, sortedEdges[0].from);
		this->graphFunction.erase(--graphFunction.end());
	}
	~GraphFunction() {//It's destructor
	}


	string getGraphFunction() {
		return graphFunction;
	}


	void printGraphFunctionInFile(string outFile = "") {
		if (!errors.empty())
			return;

		if (outFile == "")
			outFile = this->outFile;
		ofstream fout(outFile);
		if (!fout.is_open())
			throw string{ "Файл " + outFile + " не найден! \n" };

		fout << graphFunction;
		fout.close();
		cout << "\nДанные успешно сохранены в " << outFile;
	}
};



class ValueGraphFunction : protected GraphFunction {
protected:
	string cmdFile;
	map <int, string> cmds;
	double valueGF;

	void getCommands() {
		unsigned short iStart = 0, twoPoints = 0;
		string str = File::read(cmdFile) + "\n";
		for (unsigned short i = 0; i < str.length(); i++) {
			if (str[i] == ':')
				twoPoints = i;
			else if (str[i] == '\n') {
				this->cmds.insert(make_pair(stoi(str.substr(iStart, iStart - twoPoints)), str.substr(twoPoints + 2, i - twoPoints - 2)));
				iStart = i + 1;
			}
		}
	}


	double pow(double num, int k) {
		double res = 1;
		for (unsigned short i = 0; i < k; i++)
			res *= num;
		return res;
	}


	double getValue(vector <Edge> edges, int v, int vPrev = 0, double res = 0) {
		int vNext = findToAndDeleteEdge(edges, v);
		/*if (vNext == 0) {
			if (cmds[vPrev] == "+")
				return res += stoi(cmds[v]);
			else if (cmds[vPrev] == "*")
				return res = 0 ? stoi(cmds[v]) : res * stoi(cmds[i]);
			else if (cmds[vPrev] == "exp")
				return pow(EXP, res);
		}*/
		while (vNext != 0) {
			res = getValue(edges, vNext, v, res);
			vNext = findToAndDeleteEdge(edges, v);
		}

		if (cmds[vPrev] == "+")
			return res += stoi(cmds[v]);
		else if (cmds[vPrev] == "*")
			return res = 0 ? stoi(cmds[v]) : res * stoi(cmds[v]);
		else if (cmds[vPrev] == "exp")
			return pow(EXP, res);
		return 0;
	}
public:
	ValueGraphFunction(string inFile, string cmdFile, string outFile) : GraphFunction(inFile, outFile) {
		this->cmdFile = cmdFile;
		getCommands();
		vector <Edge> sortedEdges = sortEdges(this->edges);
		this->valueGF = getValue(sortedEdges, sortedEdges[0].from);

		cout << endl << this->graphFunction << endl << this->valueGF;
	}
	~ValueGraphFunction() {
	}
};



int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "ru");

	if (argc != 3 && argc != 4) {
		cout << "\nНеверно заданы параметры функции! \n";
		return 0;
	}

	string inFile = argv[1];
	string cmdFile = argv[2];
	string outFile = "out.txt";
	if (argc == 3)
		outFile = argv[3];
	
	try {
		ValueGraphFunction valueFun(inFile, cmdFile, outFile);
	}
	catch (string& error) {
		cout << error;
	}
	cout << endl;
	return 0;
}
