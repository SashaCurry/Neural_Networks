#include "iostream"
#include "vector"
#include "string"
#include "fstream"
#include "set"
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;
using namespace nlohmann;

class NeuronValue
{
public:
	double before, after;

	NeuronValue(double before, double after)
	{
		this->before = before;
		this->after = after;
	}
};



class TrainingData
{
public:
	vector <double> input, output;
	
	TrainingData(vector <double> input, vector <double> output)
	{
		this->input = input;
		this->output = output;
	}
};



class NeurealNetwork
{
private:
	vector <vector <double>> mult(vector <vector <double>> a, vector <vector <double>> b)
	{
		unsigned short rawsA = a.size();
		unsigned short colsA = a[0].size();
		unsigned short colsB = b[0].size();
		vector <vector <double>> res(rawsA, vector <double>(colsB, 0));

		for (unsigned short i = 0; i < rawsA; i++)
			for (unsigned short j = 0; j < colsB; j++)
				for (unsigned short k = 0; k < colsA; k++)
					res[i][j] += a[i][k] * b[k][j];
		return res;
	}


	vector <vector <double>> transpose(vector <vector <double>> matrix)
	{
		unsigned short raws = matrix.size();
		unsigned short cols = matrix[0].size();
		vector <vector <double>> res(cols, vector <double>(raws, 0));

		for (unsigned short i = 0; i < raws; i++)
			for (unsigned short j = 0; j < cols; j++)
				res[j][i] = matrix[i][j];
		return res;
	}
public:
	vector <int> numNeuron;
	vector <vector <vector <double>>> neuronet;
	set <string> errors;
	vector <string> outputData;

	NeurealNetwork(string matrixFile) {
		ifstream fin(matrixFile);
		if (!fin.is_open())
			throw string{ "Файл " + matrixFile + " не найден! \n" };
		json jsonMas;
		
		try
		{
			fin >> jsonMas;
		}
		catch (const json::parse_error& e)
		{
			throw string{ "Файл " + matrixFile + " не является файлом JSON-формата!" };
		}

		for (unsigned short i = 0; i < jsonMas.size(); i++)
		{
			json layerJSON = jsonMas[i];
			vector <vector <double>> layer;

			for (unsigned short j = 0; j < layerJSON.size(); j++)
			{
				json neuronJSON = layerJSON[j];
				vector <double> neuron;

				for (unsigned short k = 0; k < neuronJSON.size(); k++)
					neuron.push_back(neuronJSON[k]);

				if (numNeuron.empty())
					numNeuron.push_back(neuron.size());
				else if (numNeuron.back() != neuron.size())
				{
					errors.insert(string{ "Ошибка в файле" + matrixFile + ", строкa " + to_string(i + 1) +
										  ": несоответствие размеров нейрона и предыдущего слоя" });
			    }

				layer.push_back(neuron);
			}
			
			numNeuron.push_back(layer.size());
			neuronet.push_back(transpose(layer));
		}
	}


	void training(string trainingFile, unsigned short n)
	{
		double k = 0.01;
		ifstream fin(trainingFile);
		if (!fin.is_open())
			throw string{ "Файл " + trainingFile + " не найден! \n" };
		
		string str;
		vector <TrainingData> trData;
		unsigned short raw = 1;
		while (getline(fin, str))
		{
			json jsonObject;
			try
			{
				jsonObject = json::parse(str);
			}
			catch (json::parse_error error)
			{
				throw string{ "Файл " + trainingFile + " не является файлом JSON-формата!" };
			}

			auto inputMas = jsonObject["i"];
			vector <double> input = inputMas.get<vector <double>>();
			auto outputMas = jsonObject["o"];
			vector <double> output = outputMas.get<vector <double>>();

			if (input.size() != numNeuron[0] || output.size() != numNeuron.back())
			{
				errors.insert(string{ "Ошибка в строке " + to_string(raw) + ": неверное количестве входных\\выходных данных!" });
				raw++;
				continue;
			}

			trData.push_back(TrainingData(input, output));
		}
		fin.close();

		if (!errors.empty())
			return;
		else if (trData.empty())
		{
			errors.insert(string{ "Не найдено данных для тренировки" });
			return;
		}

		int numberTraining = 0;
		for (unsigned short i = 1; i <= n; i++)
		{
			vector <vector <NeuronValue>> neuronsValues;
			TrainingData curTrData = trData[numberTraining];
			vector <double> input = curTrData.input;
			vector <NeuronValue> neuronsValuesInLayer;

			for (double d : input)
				neuronsValuesInLayer.push_back(NeuronValue(d, d));
			neuronsValues.push_back(neuronsValuesInLayer);

			vector <vector <double>> res;
			res.push_back(input);

			for (auto m : this->neuronet)
			{
				neuronsValuesInLayer.clear();
				res = mult(res, m);

				for (unsigned short j = 0; j < res[0].size(); j++)
				{
					NeuronValue curVal(res[0][j], res[0][j] / (1 + abs(res[0][j])));
					res[0][j] = curVal.after;
					neuronsValuesInLayer.push_back(curVal);
				}

				neuronsValues.push_back(neuronsValuesInLayer);
			}

			vector <vector < vector <double>>> deltaNeuronet;
			vector <double> errors;

			for (unsigned short m = 0, j = neuronsValues.size() - 1; m < neuronsValues[j].size(); m++)
			{
				double tk = curTrData.output[m];
				double yk = neuronsValues[j][m].after;
				double help = 1.0 / ((1 + abs(neuronsValues[j][m].before)) * (1 + abs(neuronsValues[j][m].before)));
				errors.push_back((tk - yk) * help);
			}

			vector <vector <double>> deltaLayer;
			vector <vector <double>> layer = neuronet[i - 1];
			for (unsigned short m = 0; m < layer[0].size(); m++)
			{
				vector <double> deltaNeuron;
				for (unsigned short l = 0; l < deltaNeuron.size(); l++)
					deltaNeuron.push_back(k * errors[m] * neuronsValues[i - 1][l].after);
				deltaLayer.push_back(deltaNeuron);
			}
			deltaNeuronet.push_back(transpose(deltaLayer));

			for (unsigned short j = neuronsValues.size() - 2; j >= 1; j--)
			{
				layer = neuronet[j];
				vector <double> errorsIn;

				for (unsigned short m = 0; m < layer.size(); m++)
				{
					double errorInj = 0;
					for (unsigned l = 0; l < layer[m].size(); l++)
						errorInj += errors[l] * layer[m][l];
					errorsIn.push_back(errorInj);
				}

				errors.clear();
				for (unsigned short m = 0; m < neuronsValues[i].size(); m++)
				{
					double help = 1.0 / ((1 + abs(neuronsValues[j][m].before)) * (1 + abs(neuronsValues[j][m].before)));
					errors.push_back(errorsIn[m] * help);
				}

				layer = neuronet[i - 1];
				deltaLayer.clear();

				for (unsigned short m = 0; m < layer.size(); m++)
				{
					vector <double> deltaNeuron;
					for (unsigned short l = 0; l < layer[m].size(); l++)
						deltaNeuron.push_back(k * errors[l] * neuronsValues[i - 1][m].after);
					deltaLayer.push_back(deltaNeuron);
				}

				deltaNeuronet.push_back(deltaLayer);
			}

			for (unsigned short m = 0; m < neuronet.size(); m++)
			{
				layer = neuronet[m];
				deltaLayer = deltaNeuronet[neuronet.size() - m - 1];

				for (unsigned short l = 0; l < layer.size(); l++)
				{
					vector <double> neuron = layer[l];
					vector <double> deltaNeuron = deltaLayer[l];

					for (unsigned r = 0; r < neuron.size(); r++)
						neuron[r] += deltaNeuron[r];
				}
			}

			string str = "Итерация " + to_string(raw) + ": [";
			raw++;
			for (unsigned short i = 0; i < errors.size(); i++)
				str += to_string(errors[i]) + ", ";
			str.pop_back();
			str += "]";
			outputData.push_back(str);

			numberTraining = (numberTraining + 1) % trData.size();
		}
	}


	void printResultInFile(string outFile, int n)
	{
		ofstream fout(outFile);
		if (!fout.is_open())
			throw string{ "Недостаточно прав для создания файла " + outFile + "!\n" };

		for (unsigned short i = 0; i < outputData.size(); i++)
			fout << outputData[i] << endl;

		fout.close();
	}
};



int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "ru");

	if (argc != 3 && argc != 4 && argc != 5) {
		cout << "\nНеверно заданы параметры функции!";
		cout << "\nПример запуска: NN_4.exe matrix1.json training1.txt 20 outRes1.txt, где:";
		cout << "\n\tmatrix1.txt - файл с описание нейронной сети \n\ttraining.txt - файл с обучающей выборкой";
		cout << "\n\t20 - число итераций обучения";
		cout << "\n\toutRes1.txt - файл с историей 20 итераций обучения (по умолчанию outValue.txt) \n";
		return 0;
	}

	string matrixFile = argv[1];
	string trainingFile = argv[2];
	int n = stoi(argv[3]);
	string outFile = "outValue.txt";
	if (argc == 5)
		outFile = argv[4];

	try
	{
		NeurealNetwork nn(matrixFile);
		nn.training(trainingFile, n);
		nn.printResultInFile(outFile, n);
	}
	catch (string& error)
	{
		cout << endl << error;
	}

	cout << endl;
	return 0;
}