#include <iostream>
#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
#include <set>
#include <string>
#include <bitset>
#include <math.h>

#define CSV_FILE "image-files.csv"
#define STRING_SIZE 150
#define HASH_SIZE 16

namespace fs = boost::filesystem;
using namespace std;
using namespace cv;

void readPictures(const fs::path& dir, stringstream& result);
void writePictures(const fs::path& path, stringstream& data);
void generateHash(fs::path imgPath, string imgName, stringstream& hashes);
string readBatch(long bSize, fstream& file);
bool writeBatch(long bSize, long nextLine, string buffer, fs::path& dir, fstream& file);

int main() {
	string dirSt = "/Users/PhoenixQoH/Desktop/phoyos";
	//cout << "Introduce a directory:\n";
	//cin >> dirSt;
	fs::path dir(dirSt);

	if(!fs::exists(dir) || fs::is_regular_file(dir)) {
		cout << "The directory is incorrect.\n";
		return -1;
	}

	fs::path csvPath(dirSt);
	csvPath /= CSV_FILE;

	if (!fs::is_regular_file(csvPath)) {
		stringstream fileNames;
		readPictures(dir, fileNames);
		writePictures(csvPath, fileNames);
	}

	fstream file;
	file.open(csvPath.string(), ios::in|ios::out);
	long nextLine;
	file >> nextLine;

	if (nextLine == -1) {
		cout << "All the contents of the file have been processed." << endl;
		return 0;
	}

	long bSize;
	cout << "Introduce how many files to process: " << endl;
	cin >> bSize;

	//Move the pointer of the file to the line of the first picture to compute the hash
	file.seekg(sizeof(char) * STRING_SIZE * nextLine + 1, file.beg);

	string buffer;
	char end;
	bool eof;
	int nDigits;
	do {
		buffer = readBatch(bSize, file);
		eof = writeBatch(bSize, nextLine, buffer, dir, file);
		nextLine += bSize;
		if (eof) {
			cout << "All the contents of the file have been processed." << endl;
			nDigits = 2;
			nextLine = -1;
			break;
		}
		do {
			cout << "Do you want to continue? (y/n)" << endl;
			cin >> end;
		} while(end != 'y' && end != 'n');
	} while(end == 'y');

	//Write the next line to process
	file.seekg(0, file.beg);

	if(!eof) {
		nDigits = floor(log10(nextLine));
	}

	file << nextLine << ',' << string(STRING_SIZE - nDigits - 2, ' ');
	file.close();

	return 0;
}

string readBatch(long bSize, fstream& file) {
	cout << "Reading batch..." << endl;
	//Read batch size number of files
	size_t buffSize = STRING_SIZE * bSize;
	char * buffC = new char[buffSize];
	file.read(buffC, buffSize);
	buffC[buffSize] = '\0';
	string buffer(buffC);
	return buffer;
}

bool writeBatch(long bSize, long nextLine, string buffer, fs::path& dir, fstream& file) {
	cout << "Writing batch..." << endl;
	//Get the name of each picture and compute its hash
	stringstream hashes;
	int begin = 0;
	int count = 0;
	size_t indx = buffer.find(',', begin);
	string fileName;
	while (count < bSize && indx != string::npos) {
		fileName = buffer.substr(begin, indx - begin);
		generateHash(dir,fileName,hashes);
		count++;
		begin = count * STRING_SIZE;
		indx = buffer.find(',', begin);
	}

	//Move the pointer to the writing position of the first image of the batch
	file.seekg(sizeof(char) * STRING_SIZE * (nextLine + 1) - (HASH_SIZE + 1) , file.beg);

	//Write the hashes
	string var;
	hashes >> var;

	while(hashes) {
		file << var;
		hashes >> var;
		file.seekg(sizeof(char) * (STRING_SIZE + 1) - (HASH_SIZE + 2) , file.cur);
	}

	//Put the file pointer back to the beginning of the line
	file.seekg(-sizeof(char) * STRING_SIZE + HASH_SIZE + 2, file.cur);
	return count < bSize;
}

void generateHash(fs::path imgPath, string imgName, stringstream& hashes) {
	imgPath /= imgName;

	Mat initMat = imread(imgPath.string(), IMREAD_GRAYSCALE);
	Size size(9,8);
	Mat imgMat;
	string hexValue;
	stringstream aux;
	resize(initMat,imgMat,size);

	uchar prev;
	bitset<64> bits;
	for (int i = 0; i<imgMat.rows; i++) {
		prev = imgMat.at<uchar>(i, 0);
		for (int j = 1; j < imgMat.cols; j++) {
			if (imgMat.at<uchar>(i, j) > prev) {
				bits.set(i * (imgMat.cols - 1) + j - 1);
			}
			prev = imgMat.at<uchar>(i, j);
		}
	}

	aux << hex << bits.to_ulong();
	aux >> hexValue;
	hashes << string(HASH_SIZE - hexValue.size(),'0') << hexValue << ",\n";
}

void writePictures(const fs::path& path, stringstream& data) {
	cout << "Writing images..." << endl;
	ofstream file;
	file.open(path.string().c_str(), ios::app);
	file << "1," + string(STRING_SIZE - 2, ' ') << endl;
	file << data.str();
	file.flush();
	file.close();
}

void readPictures(const fs::path& dir, stringstream& result) {
	cout << "Reading images..." << endl;
	std::set<std::string> exts;
	exts.insert(".png");
	exts.insert(".jpg");
	exts.insert(".jpeg");
	exts.insert(".bmp");

	fs::directory_iterator it(dir);
	fs::directory_iterator endit;

	fs::path p;
	string e;
	string name;
	while(it != endit) {
		p = it->path();
		e = p.extension().string();

		if(fs::is_regular_file(*it) && exts.find(e) != exts.end()) {
			name = p.stem().string() + e;
			result << name + ',' + string(STRING_SIZE - name.size() - 2, ' ') + '\n';
		}
		it++;
	}
}
