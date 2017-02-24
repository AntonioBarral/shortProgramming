#include <iostream>
#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
#include <set>
#include <string>

#define CSV_FILE "image-files.csv"
#define STRING_SIZE 14

namespace fs = boost::filesystem;
using namespace std;
using namespace cv;

void getPictures(const fs::path& dir, vector<string>& result);
void writePictures(const fs::path& path, vector<string>& data);
string generateHash(fs::path& imgPath);

int main() {
	string dirSt;
	cout << "Introduce a directory:\n";
	cin >> dirSt;
	fs::path dir(dirSt);

	if(!fs::exists(dir) || !fs::is_directory(dir)) {
		cout << "The directory is incorrect.\n";
		return -1;
	}

	fs::path csvPath(dir);
	csvPath /= CSV_FILE;

	if (!fs::exists(csvPath)) {
		cout << "Reading images...\n";
		vector<string> fileNames;
		getPictures(dir, fileNames);
		writePictures(csvPath, fileNames);
	}

	//FILE *file = fopen(csvPath.string().c_str(), "r+");
	//long l;
	//fscanf(file, "%lu\n", &l);
	//fseek(file, (l - 1)* STRING_SIZE + (l / 10 + 1), SEEK_SET);
	//fputs("a", file);
	return 0;
}

string generateHash(fs::path& imgPath) {
	Mat initMat = imread(imgPath.string(), IMREAD_REDUCED_GRAYSCALE_8);
	Size size(9,8);
	Mat imgMat;

	resize(initMat,imgMat,size);

	uchar prev;
	bitset<64> bits;
	for (int i = 0; i<imgMat.rows; i++) {
		prev = imgMat.at<uchar>(i, 0);
		for (int j = 1; j < imgMat.cols; j++) {
			if (imgMat.at<uchar>(i, j) > prev) {
				bits.set(i * (imgMat.cols - 1) + j - 1);
			}
		}
	}

	std::stringstream ss;
	string hash;
	ss << hex << bits.to_ulong() << endl;
	ss >> hash;

	return hash;
}

void writePictures(const fs::path& path, vector<string>& data) {
	FILE *file = fopen(path.string().c_str(), "w");
	fwrite("4\n", sizeof(char), 2, file);
	fwrite(data.data(), sizeof(string), data.size(), file );
	fclose(file);
}

void getPictures(const fs::path& dir, vector<string>& result) {
	std::set<std::string> exts;
	exts.insert(".png");
	exts.insert(".jpg");
	exts.insert(".jpeg");
	exts.insert(".gif");
	exts.insert(".bmp");

	fs::directory_iterator it(dir);
	fs::directory_iterator endit;

	fs::path p;
	string e;
	string name;
	while(it != endit) {
		p = it->path();
		cout << p.string() << endl;
		e = p.extension().string();

		if(fs::is_regular_file(*it) && exts.find(e) != exts.end()) {
			name = p.stem().string() + e;
			result.push_back(name + ',' + string(STRING_SIZE - name.size() - 2, ' ') + '\n');
		}
		it++;
	}
}
