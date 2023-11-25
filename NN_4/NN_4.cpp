#include "iostream"
#include "vector"
#include "string"
#include "fstream"
#include "set"

using namespace std;

class File
{
public:
	static string read(string inFile) {
		ifstream fin(inFile);
		if (!fin.is_open())
			throw string{ "Файл " + inFile + " не найден! \n" };

		string str = "";
		for (char el; fin.get(el);)
			str += el;
		if (str.back() != '\n')
			str += '\n';

		fin.close();
		return str;
	}

	static void write(string str, string outFile) {
		ofstream fout(outFile);
		if (!fout.is_open())
			throw string{ "Недостаточно прав для создания файла " + outFile + "!\n" };

		fout << str;
		if (str.back() != '\n')
			fout << "\n";
		fout.close();
	}

	static void append(string str, string outFile) {
		ofstream fout(outFile, ios::app);
		if (!fout.is_open())
			throw string{ "Файл " + outFile + " не найден! \n" };

		fout << endl << str;
		if (str.back() != '\n')
			fout << '\n';
		fout.close();
	}
};



class NeurealNetwork
{
private:
	bool checkStrDigit(string str, short raw, string file)
	{
		if (str.empty()) {
			errors.insert(string{ "Ошибка в файле " + file + ", " + "строкa " + to_string(raw) + ": некорретные данные(введены символы вместо цифр)" });
			return false;
		}
		for (int i = 0; i < str.length(); i++)
			if (!isdigit(str[i])) {
				errors.insert(string{ "Ошибка в файле " + file + ", " + "строкa " + to_string(raw) + ": некорретные данные(введены символы вместо цифр)" });
				return false;
			}
		return true;
	}
public:
	vector <vector <vector <int>>> matrix;
	vector <double> vec;
	set <string> errors;
	string matrixFile, vectorFile;
	vector <double> valueNN;

	NeurealNetwork(string matrixFile, string vectorFile)
	{
		this->matrixFile = matrixFile;
		this->vectorFile = vectorFile;
		getData(matrixFile, vectorFile);
		checkErrors();
		getValueNN();
	}


	void getData(string matrixFile, string vectorFile)
	{
		matrix.clear(), vec.clear();

		string str = File::read(matrixFile);
		vector <vector <int>> raw;
		string substr = "";
		unsigned short numStr = 1;
		for (unsigned short i = 0; i < str.size(); i++)
		{
			switch (str[i])
			{
			case '[':
				substr = "";
				raw.push_back(vector <int> {});
				break;
			case ']':
				if (!checkStrDigit(substr, numStr, matrixFile))
					continue;
				raw.back().push_back(stoi(substr));
				break;
			case ' ':
				if (str[i - 1] == ']' || !checkStrDigit(substr, numStr, matrixFile))
					continue;
				raw.back().push_back(stoi(substr));
				substr = "";
				break;
			case '\n':
				matrix.push_back(raw);
				raw.clear();
				numStr++;
				break;
			default:
				substr += str[i];
			}
		}

		str = File::read(vectorFile);
		substr = "";
		for (unsigned short i = 0; i < str.size(); i++)
		{
			if (str[i] == ' ' || str[i] == '\n')
			{
				if (!checkStrDigit(substr, numStr, vectorFile))
					continue;
				vec.push_back(stoi(substr));
				substr = "";
			}
			else
				substr += str[i];
		}
	}


	void checkErrors()
	{
		unsigned short vecSize = vec.size();
		for (unsigned short i = 0; i < matrix[0].size(); i++)
			if (matrix[0][i].size() != vecSize)
				errors.insert(string{ "Ошибка в файле " + matrixFile + ", " + "строкa 1: разное количество элементов нейрона и вектора" });

		for (unsigned short i = 1; i < matrix.size(); i++)
		{
			for (unsigned short j = 0; j < matrix[i].size(); j++)
			{
				if (matrix[i][j].size() != matrix[i - 1].size())
				{
					errors.insert(string{ "Ошибка в файле" + matrixFile + ", строкa " + to_string(i + 1) +
										  ": несоответствие размеров нейрона и предыдущего слоя" });
				}
			}
		}
	}


	vector <double> getValueNN() {
		if (!errors.empty())
		{
			for (auto i = errors.begin(); i != errors.end(); i++)
				cout << endl << *i;
			return vector <double> {-1};
		}

		vector <double> res = this->vec;
		vector <double> vec;
		for (unsigned short i = 0; i < matrix.size(); i++)
		{
			vec = res;
			res.clear();
			for (unsigned short j = 0; j < matrix[i].size(); j++)
			{
				double val = 0;
				for (unsigned short k = 0; k < matrix[i][j].size(); k++)
					val += matrix[i][j][k] * vec[k];
				res.push_back(val / (1 + abs(val)));
			}
		}
		this->valueNN = res;
		return res;
	}


	void printResultInFile(string outNetFile = "outNetwork.txt", string outValFile = "outValue.txt")
	{
		ofstream fout(outNetFile);
		if (!fout.is_open())
			throw string{ "Недостаточно прав для создания файла " + outNetFile + "!\n" };

		fout << "<network>";
		for (unsigned short i = 0; i < matrix.size(); i++)
		{
			fout << "\n\t<layer>";
			for (unsigned short j = 0; j < matrix[i].size(); j++)
			{
				fout << "\n\t\t<connections>";
				for (unsigned short k = 0; k < matrix[i][j].size(); k++)
					fout << "\n\t\t\t<weight>" << matrix[i][j][k] << "</weight>";
				fout << "\n\t\t</connections>";
			}
			fout << "\n\t</layer";
		}
		fout << "\n</network>";
		cout << "\nМногослойная нейронная сеть успешно записана в файл " << outNetFile;
		fout.close();

		fout.open(outValFile);
		if (!fout.is_open())
			throw string{ "Недостаточно прав для создания файла " + outValFile + "!\n" };

		for (unsigned short i = 0; i < valueNN.size(); i++)
			fout << valueNN[i] << " ";
		cout << "\nРезультат вычислений НС успешно записан в файл " << outValFile;
		fout.close();
	}
};

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "ru");
	
	if (argc != 3 && argc != 4 && argc != 5) {
		cout << "\nНеверно заданы параметры функции!";
		cout << "\nПример запуска: NN_4.exe matrix1.txt vector1.txt outNetwork1.txt outValue1.txt, где:";
		cout << "\n\tmatrix1.txt - файл с матрицей \n\tvector1.txt - файл с вектором";
		cout << "\n\toutNetwork1.txt - многослойная нейронная сеть в формате XML (по умолчанию outNetwork.txt)";
		cout << "\n\toutValue1.txt - результат вычислений нейронной сети (по умолчанию outValue.txt) \n";
		return 0;
	}

	string matrixFile = argv[1];
	string vectorFile = argv[2];
	string outNetworkFile = "outNetwork.txt";
	string outValueFile = "outValue.txt";
	if (argc >= 4)
		outNetworkFile = argv[3];
	if (argc == 5)
		outValueFile = argv[4];

	try
	{
		NeurealNetwork nn(matrixFile, vectorFile);
		nn.printResultInFile(outNetworkFile, outValueFile);
	}
	catch (string& errors)
	{
		cout << endl << errors;
	}
	
	cout << endl;
	return 0;
}