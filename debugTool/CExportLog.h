#pragma once
using namespace std;
#include <vector>

class CExportLog
{
public:
	CExportLog();

	//sharing database
	int dbThreadProgress;//Renew by timer

	//get ini properties method
	CString getInputLocation() { return strInputLocation; };
	CString getOutputLocation() { return strOutputLocation; };
	int getLogSize() { return iLogSize; };
	CString getstrLogSize() { return strLogSize; };
	BOOL getConcentrate() { return isConcentrate; }

	//get ini info method
	void setIniFilePathToBuffer(wchar_t* Buffer, const wchar_t* IniName);
	CString getInputLocationAtIni(CString strFilePath);
	CString getOutputLocationAtIni(CString strFilePath);
	CString getStrLogSizeAtIni(CString strFilePath);
	CString selectInputFilePath();
	int getIntLogSizeAtIni(CString strFilePath);
	BOOL isDefault(CString strContent);
	BOOL getboolAtIni(CString strFilePath);
	vector<vector<CString>> getVectorAboutAllTargetWord(CString strInputLocation);

	//export 
	void startExport();
	static DWORD WINAPI ExportThread(LPVOID lpParam);
	vector<CString> getVectorAboutIniFilesAfterTraverseAndSort(const CString& strInputLocation);
	void ExportNewLog(CString strPath);
	void ExportConcentratedNewLog(CString strPath);

private:
	//save ini properties
	vector<vector<CString>>vVstrTargetWord;
	CString strInputLocation;
	CString strOutputLocation;
	int iLogSize;
	CString strLogSize;
	BOOL isConcentrate;
	//export properties
	HANDLE h_ThreadProgress;
};


