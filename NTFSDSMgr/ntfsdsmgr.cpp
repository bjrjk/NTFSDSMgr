#include<iostream>
#include<fstream>
#include<cstdlib>
#include<cstdio>
#include<cstring>
#include<codecvt>
#include<vector>
#include<locale>
#include<string>
#include<windows.h>
#include<fileapi.h>
using namespace std;

class MDSFile {
private:
	wstring filePath;
	wstring streamName;
public:
	struct StreamInfo {
		long long size;
		wstring name;
		StreamInfo(long long size, wstring name) :size(size), name(name) {}
	};
	MDSFile(const wstring& filePath) :filePath(filePath), streamName(L"") {}
	MDSFile(const wstring& filePath, const wstring& streamName) :filePath(filePath), streamName(streamName) {}
	string getLastError() const{
		DWORD errorNo = GetLastError();
		string HANDLE_EOF = "No streams can be found";
		string INVALID_PARAMETER = "Current filesystem does not support streams";
		string UNKNOWN_REASON = "Unknown error ";
		if (errorNo == ERROR_HANDLE_EOF) return HANDLE_EOF;
		else if (errorNo == ERROR_INVALID_PARAMETER) return INVALID_PARAMETER;
		else return UNKNOWN_REASON + to_string(errorNo);
	}
	vector<StreamInfo> getStreamInfo() const{
		HANDLE streamNo = INVALID_HANDLE_VALUE;
		WIN32_FIND_STREAM_DATA streamData;
		vector<StreamInfo> result;
		streamNo = FindFirstStreamW(filePath.c_str(), FindStreamInfoStandard, &streamData, 0);
		if (streamNo == INVALID_HANDLE_VALUE) goto ERR;
		while (1) {
			result.push_back(StreamInfo(streamData.StreamSize.QuadPart, streamData.cStreamName));
			BOOL flag = FindNextStreamW(streamNo, &streamData);
			if (!flag) {
				if (GetLastError() == ERROR_HANDLE_EOF)break;
				else goto ERR;
			}
		}
		return result;
	ERR:
		return vector<StreamInfo>();
	}
	void setStreamName(const wstring& streamName) {
		this->streamName = streamName;
	}
	void copyFrom(const MDSFile& src) {
		MDSFile& dst = *this;
		const wstring& srcFileName = src.filePath;
		const wstring& srcFileStream = src.streamName;
		const wstring& dstFileName = dst.filePath;
		const wstring& dstFileStream = dst.streamName;
		ifstream inFile(srcFileName + L":" + srcFileStream + L":$DATA", ios::binary | ios::in);
		if (!inFile) return;
		ofstream outFile(dstFileName + L":" + dstFileStream + L":$DATA", ios::binary | ios::out);
		if (!outFile) return;
		char c;
		while (inFile.get(c))
			outFile.put(c);
		outFile.close();
		inFile.close();
	}
};

void copy(wstring srcFileName, wstring srcFileStream, wstring dstFileName, wstring dstFileStream) {
	MDSFile src(srcFileName, srcFileStream);
	MDSFile dst(dstFileName, dstFileStream);
	dst.copyFrom(src);
}
void display(wstring fileName) {
	MDSFile f(fileName);
	auto streamDataArray = f.getStreamInfo();
	wcout << "File: " << fileName << endl;
	wcout << "StreamSize, StreamName" << endl;
	for (auto& streamData : streamDataArray) {
		wcout << streamData.size << ", " << streamData.name << endl;
	}
}
void help() {
	vector<string> prompts = {
		"NTFS Data Stream Manager -- NTFSDSMgr",
		"Command:",
		"NTFSDSMgr help // Show Help Information",
		"NTFSDSMgr display [fileName] //Display Data Stream for [fileName]",
		"NTFSDSMgr copy [srcFileName] [srcFileStream] [dstFileName] [dstFileStream] //Copy Data from [srcFileName]:[srcFileStream] to [dstFileName]:[dstFileStream]",
		"Example:",
		"NTFSDSMgr display \"test.txt\"",
		"NTFSDSMgr copy \"src.txt\" \"srcstream\" \"dst.txt\" \"dststream\""
	};
	for (auto& prompt : prompts) {
		cout << prompt << endl;
	}
}
wstring str2wstr(const string& s) {
	wstring_convert<codecvt_utf8<wchar_t>> conv;
	return conv.from_bytes(s);
}
int main(int argc, char* argv[]) {
	wcout.imbue(locale("chs"));
	if (argc <= 1)help();
	else if (strcmp(argv[1], "help") == 0)help();
	else if (strcmp(argv[1], "display") == 0)display(str2wstr(argv[2]));
	else if (strcmp(argv[1], "copy") == 0)copy(str2wstr(argv[2]), str2wstr(argv[3]), str2wstr(argv[4]), str2wstr(argv[5]));
	else help();
}