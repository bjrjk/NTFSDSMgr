#include<iostream>
#include<cstdlib>
#include<cstdio>
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
public:
	struct StreamInfo {
		long long size;
		wstring name;
		StreamInfo(long long size, wstring name) :size(size), name(name) {}
	};
	MDSFile(const wstring& filePath) :filePath(filePath) {}
	string getLastError() {
		DWORD errorNo = GetLastError();
		string HANDLE_EOF = "No streams can be found";
		string INVALID_PARAMETER = "Current filesystem does not support streams";
		string UNKNOWN_REASON = "Unknown error ";
		if (errorNo == ERROR_HANDLE_EOF) return HANDLE_EOF;
		else if (errorNo == ERROR_INVALID_PARAMETER) return INVALID_PARAMETER;
		else return UNKNOWN_REASON + to_string(errorNo);
	}
	vector<StreamInfo> getStreamInfo() {
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
};
wstring str2wstr(const string& s) {
	wstring_convert<codecvt_utf8<wchar_t>> conv;
	return conv.from_bytes(s);
}
int main(int argc, char* argv[]) {
	wcout.imbue(locale("chs"));
	if (argc != 2)return 1;
	wstring fileName = str2wstr(argv[1]);
	MDSFile f(fileName);
	auto streamDataArray = f.getStreamInfo();
	wcout << "File: " << fileName << endl;
	for (auto& streamData : streamDataArray) {
		wcout << streamData.size << " " << streamData.name << endl;
	}
}