#include "pch.h"
#include "CExportLog.h"

CExportLog::CExportLog() {
	vVstrTargetWord.clear();
	strInputLocation = L"";
	strOutputLocation = L"";
	iLogSize = 0;
	strLogSize = L"";
	isConcentrate = false;
	h_ThreadProgress = NULL;
	dbThreadProgress = 1;
}

//get ini data
void CExportLog::setIniFilePathToBuffer(wchar_t* Buffer, const wchar_t* IniName) {
	wchar_t strFilePath[MAX_PATH];
	//get the path og exe. Because config,ini is locate  there
	GetModuleFileName(NULL, strFilePath, MAX_PATH);
	PathRemoveFileSpec(strFilePath);
	wcscat_s(strFilePath, L"\\");
	wcscat_s(strFilePath, IniName);
	memcpy(Buffer, strFilePath, MAX_PATH);
}

CString CExportLog::getInputLocationAtIni(CString strFilePath) {
	CString strContentReturn;
	GetPrivateProfileString(L"input", L"InputLocation", L"Default", strContentReturn.GetBuffer(256), 256, strFilePath);

	while (isDefault(strContentReturn) || (strContentReturn == L"")) {
		//make sure repeat click would not happen
		strContentReturn = selectInputFilePath();
	}

	strInputLocation = strContentReturn;
	strContentReturn.ReleaseBuffer();
	return strInputLocation;
}

CString CExportLog::getOutputLocationAtIni(CString strFilePath) {
	CString strContentReturn;
	GetPrivateProfileString(L"output", L"OutputLocation", L"Default", strContentReturn.GetBuffer(256), 256, strFilePath);
	
	strOutputLocation = strContentReturn;
	strContentReturn.ReleaseBuffer();
	return strOutputLocation;
}

CString CExportLog::selectInputFilePath() {
	CString strFolderPath;
	BROWSEINFO bi = { 0 };
	bi.ulFlags = BIF_NEWDIALOGSTYLE | BIF_EDITBOX | BIF_RETURNONLYFSDIRS;
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

	if (pidl != NULL)
	{
		TCHAR path[MAX_PATH];
		if (SHGetPathFromIDList(pidl, path))
		{
			strFolderPath = path;
		}
		CoTaskMemFree(pidl);
	}
	return strFolderPath;
}

CString CExportLog::getStrLogSizeAtIni(CString strFilePath) {
	CString strContentReturn;
	GetPrivateProfileString(L"output", L"LogSize", L"Default", strContentReturn.GetBuffer(256), 256, strFilePath);
	strLogSize = strContentReturn;
	strContentReturn.ReleaseBuffer();
	return strLogSize;
}

int CExportLog::getIntLogSizeAtIni(CString strFilePath) {
	iLogSize = GetPrivateProfileInt(L"output", L"LogSize", 0, strFilePath);
	return iLogSize;
}

BOOL CExportLog::isDefault(CString strContent) {
	if (strContent == L"Default") {
		return true;
	}
	return false;
}

vector<vector<CString>> CExportLog::getVectorAboutAllTargetWord(CString strInputLocation) {
	vector<CString>vAndTargetWord;
	vAndTargetWord.clear();
	vector<vector<CString>>vvOrTargetWord;
	vvOrTargetWord.clear();

	CStdioFile fileRead;
	if (!fileRead.Open(strInputLocation, CFile::modeRead)) {
		AfxMessageBox(L"The file is opening or other error. Create fail!");
		vvOrTargetWord.clear();
		return vvOrTargetWord;
	}

	CString isTargetWord;
	CString strRead;
	int iFirstDoubleQuotesFind = 0;
	int iSecondDoubleQuotesFind = -1;
	fileRead.ReadString(isTargetWord);
	if (isTargetWord == L"[target_words]") {
		while (fileRead.ReadString(strRead)) {
			if (strRead.Find(L"[") == -1) {//have already read the next title, go to else and break
				while ((strRead.Find(L"word = ") != -1)) {//follow the ini rule
					iFirstDoubleQuotesFind = strRead.Find(L"\"", iSecondDoubleQuotesFind + 1);
					iSecondDoubleQuotesFind = strRead.Find(L"\"", iFirstDoubleQuotesFind + 1);
					if (iFirstDoubleQuotesFind != -1 && iSecondDoubleQuotesFind != -1) {
						vAndTargetWord.push_back(strRead.Mid(iFirstDoubleQuotesFind + 1, iSecondDoubleQuotesFind - iFirstDoubleQuotesFind - 1));
					}
					else {
						iFirstDoubleQuotesFind = 0;
						iSecondDoubleQuotesFind = -1;
						break;
					}
				}
				vvOrTargetWord.push_back(vAndTargetWord);
				vAndTargetWord.clear();
			}
			else {
				break;
			}
		}
	}

	fileRead.Close();
	vVstrTargetWord = vvOrTargetWord;
	return vvOrTargetWord;
}

BOOL CExportLog::getboolAtIni(CString strFilePath) {
	int value = 0;
	value = GetPrivateProfileInt(L"output", L"Concentrate", 0, strFilePath);
	if (value == 0) {
		isConcentrate = false;
	}
	else {
		isConcentrate = true;
	}
	return isConcentrate;
}


//export process
void CExportLog::startExport() {
	//get ini content from ini
	wchar_t strFilePath[MAX_PATH];
	setIniFilePathToBuffer(strFilePath, L"Config.ini");
	getInputLocationAtIni(strFilePath);
	getOutputLocationAtIni(strFilePath);
	getStrLogSizeAtIni(strFilePath);
	getIntLogSizeAtIni(strFilePath);
	getVectorAboutAllTargetWord(strFilePath);
	getboolAtIni(strFilePath);
	if ((strInputLocation == L"")||(strOutputLocation == L"")||(iLogSize == 0) || (strLogSize == L"")) {
		AfxMessageBox(L"set ini error! startExport() fail!");
		return;
	}

	dbThreadProgress = 1;
	if (h_ThreadProgress) {
		WaitForSingleObject(h_ThreadProgress, INFINITE);
		h_ThreadProgress = NULL;
		h_ThreadProgress = CreateThread(NULL, 0, ExportThread, this, 0, NULL);
	}
	else {
		h_ThreadProgress = CreateThread(NULL, 0, ExportThread, this, 0, NULL);
	}
}

DWORD WINAPI CExportLog::ExportThread(LPVOID lpParam) {
	CExportLog* cel = (CExportLog*)lpParam;
	vector<CString> vFilePathSortByTheLastModifiedTime;
	vFilePathSortByTheLastModifiedTime.clear();

	Sleep(500);

	CString strInputLocation = cel->getInputLocation();
	BOOL isConcentrate = cel->getConcentrate();
	int iProgressStep = 100;
	if (strInputLocation.Find(L".log") == -1) {//判斷路徑是設定在資料夾路徑還是檔案路徑
		vFilePathSortByTheLastModifiedTime = cel->getVectorAboutIniFilesAfterTraverseAndSort(strInputLocation);
		iProgressStep = 100 / vFilePathSortByTheLastModifiedTime.size();
		if (vFilePathSortByTheLastModifiedTime.empty() == 1) {
			AfxMessageBox(L"The path doesn't have .log");
			return 0;
		}

		for (int i = 0; i < vFilePathSortByTheLastModifiedTime.size(); i++) {
			if (isConcentrate) {
				cel->ExportConcentratedNewLog(vFilePathSortByTheLastModifiedTime[i]);
			}
			else {
				cel->ExportNewLog(vFilePathSortByTheLastModifiedTime[i]);
			}
			cel->dbThreadProgress += iProgressStep;
		}
	}
	else {
		vFilePathSortByTheLastModifiedTime.push_back(strInputLocation);
		if (isConcentrate) {
			cel->ExportConcentratedNewLog(vFilePathSortByTheLastModifiedTime[0]);
		}
		else {
			cel->ExportNewLog(vFilePathSortByTheLastModifiedTime[0]);

		}
		cel->dbThreadProgress += iProgressStep;
	}
	return 1;
}

vector<CString> CExportLog::getVectorAboutIniFilesAfterTraverseAndSort(const CString& strInputLocation) {
	vector<CString> vFilePathSortByTheLastModifiedTime;
	vFilePathSortByTheLastModifiedTime.clear();
	vector<CTime>vFileLastModifiedTime;
	vFileLastModifiedTime.clear();

	CFileFind finder;
	CString strWildcard = strInputLocation + "\\*.log";
	BOOL bWorking = finder.FindFile(strWildcard);
	while (bWorking)
	{
		bWorking = finder.FindNextFile();

		if (finder.IsDots()) {
			continue;
		}

		if (finder.IsDirectory()) {
			//Dno't know why recursive, maybe would happen some issue
			getVectorAboutIniFilesAfterTraverseAndSort(finder.GetFilePath());
		}

		CString strFilePath = finder.GetFilePath();
		CTime lastModifiedTime;
		finder.GetLastWriteTime(lastModifiedTime);

		if (vFileLastModifiedTime.empty()) {
			vFilePathSortByTheLastModifiedTime.push_back(strFilePath);
			vFileLastModifiedTime.push_back(lastModifiedTime);
		}
		else if (vFileLastModifiedTime.size() == 1) {
			if (lastModifiedTime < vFileLastModifiedTime[0]) {
				//place at the back
				vFilePathSortByTheLastModifiedTime.push_back(strFilePath);
				vFileLastModifiedTime.push_back(lastModifiedTime);
			}
			else {
				//place front
				vector<CString>::iterator itFilePath = vFilePathSortByTheLastModifiedTime.begin();
				vector<CTime>::iterator itModifiedTime = vFileLastModifiedTime.begin();
				vFilePathSortByTheLastModifiedTime.insert(itFilePath, strFilePath);
				vFileLastModifiedTime.insert(itModifiedTime, lastModifiedTime);
			}
		}
		else {
			vector<CString>::iterator itFilePath = vFilePathSortByTheLastModifiedTime.begin();
			vector<CTime>::iterator itModifiedTime = vFileLastModifiedTime.begin();

			while (itModifiedTime != vFileLastModifiedTime.end()) {
				if (lastModifiedTime >= *itModifiedTime) {
					//place at the back
					vFilePathSortByTheLastModifiedTime.insert(itFilePath, strFilePath);
					vFileLastModifiedTime.insert(itModifiedTime, lastModifiedTime);
					break;
				}
				else if (itModifiedTime == vFileLastModifiedTime.end() - 1) {
					//place front
					vFilePathSortByTheLastModifiedTime.push_back(strFilePath);
					vFileLastModifiedTime.push_back(lastModifiedTime);
					break;
				}
				else {
					itModifiedTime++;
					itFilePath++;
				}
			}
		}
	}
	return vFilePathSortByTheLastModifiedTime;
}

void CExportLog::ExportNewLog(CString strPath) {
	//open the old log
	CStdioFile fileRead;
	CString strInputfilePath = strPath;

	if (!fileRead.Open((LPCTSTR)strInputfilePath, CFile::modeReadWrite | CFile::modeNoTruncate)) {
		AfxMessageBox(L"The file is opening or doesn't exit, please check ini!");
		return;
	}

	//get file name and set new log name
	CFileFind finder;
	BOOL bWorking = finder.FindFile(strInputfilePath);
	bWorking = finder.FindNextFile();
	CString NewfileName = +L"NewLogOf" + finder.GetFileName();

	//create the new log
	CStdioFile fileWrite;
	CString strOutPutfilePath = strOutputLocation + L"\\" + NewfileName;
	if (!fileWrite.Open(strOutPutfilePath, CFile::modeCreate | CFile::modeWrite)) {
		AfxMessageBox(L"The file is opening or other error. Create fail!");
		return;
	}

	//set the limit size
	ULONGLONG fileSizeLimit = iLogSize * 1024 * 1024;

	//set if new log over 50MB
	CFileStatus fileStatus;
	int index = 1;
	CString extendNewfileName;
	CString extendNewfilePath;
	CString strRead;

	BOOL isConcentrate = getConcentrate();

	while (fileRead.ReadString(strRead)) {//read and get the old log string which has the keyword
		for (auto itVV = vVstrTargetWord.begin(); itVV < vVstrTargetWord.end(); ) {
			if ((*itVV).size() <= 0) {
				itVV++;
				continue;
			}
			else {
				for (auto itV = (*itVV).begin(); itV < (*itVV).end(); itV++) {//比對AND內的TargetWord
					if (strRead.Find(*itV) != -1) {//比較是否有TagetWord
						if (itV == (*itVV).end() - 1) {//確認是否是最後一個TargetWord，是的話寫入
							fileWrite.GetStatus(fileStatus);

							ULONGLONG currentFileSize = fileStatus.m_size;
							
							if (currentFileSize < fileSizeLimit) {
								fileWrite.SeekToEnd();
								fileWrite.WriteString(strRead + L"\n");
							}
							else {//if over limit size
								fileWrite.Close();
								extendNewfileName.Format(_T("%s_%d.log"), NewfileName, index);
								extendNewfilePath = strOutputLocation + L"\\" + extendNewfileName;
								if (!fileWrite.Open(extendNewfilePath, CFile::modeCreate | CFile::modeWrite))
								{
									AfxMessageBox(L"Write " + extendNewfilePath + L"Fail! The file is opening or other error!");
									return;
								}
								index++;
								fileWrite.WriteString(strRead + L"\n");
							}
							//因為是最後一個TargetWord，寫入後，讀取下一行-->刷回最原本的迴圈設定
							fileRead.ReadString(strRead);
							itVV = vVstrTargetWord.begin();
							break;
						}
						else {//不是最後一個TargetWord，換下一個TargetWord比較
							continue;
						}
					}
					else {
						itVV++;
						break;//沒有符合AND，換下一個Or比較
					}
				}
			}
		}
	}
	fileWrite.Close();
	fileRead.Close();
}

void CExportLog::ExportConcentratedNewLog(CString strPath) {
	//open the old log
	CStdioFile fileRead;
	CString strInputfilePath = strPath;

	if (!fileRead.Open((LPCTSTR)strInputfilePath, CFile::modeReadWrite | CFile::modeNoTruncate)) {
		AfxMessageBox(L"The file is opening or doesn't exit, please check ini!");
		return;
	}

	//get file name and set new log name
	CString NewfileName = L"ConcentratedLog.log";

	//create the new log
	CStdioFile fileWrite;
	CString strOutPutfilePath = strOutputLocation + L"\\" + NewfileName;
	if (!fileWrite.Open(strOutPutfilePath, CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate)) {
		AfxMessageBox(L"The file is opening or other error. Create fail!");
		return;
	}
	fileWrite.SeekToEnd();
	fileWrite.WriteString(L"file source path : " + strInputfilePath + L"\n");//創建檔案完，立刻寫入原始檔案路徑

	//set the limit size
	ULONGLONG fileSizeLimit = iLogSize * 1024 * 1024;

	//set if new log over 50MB
	CString strRead;

	while (fileRead.ReadString(strRead)) {//read and get the old log string which has the keyword
		for (auto itVV = vVstrTargetWord.begin(); itVV < vVstrTargetWord.end(); ) {
			if ((*itVV).size() <= 0) {
				itVV++;
				continue;
			}
			else {
				for (auto itV = (*itVV).begin(); itV < (*itVV).end(); itV++) {//比對AND內的TargetWord
					if (strRead.Find(*itV) != -1) {//比較是否有TagetWord
						if (itV == (*itVV).end() - 1) {//確認是否是最後一個TargetWord，是的話寫入

							fileWrite.SeekToEnd();
							fileWrite.WriteString(strRead + L"\n");

							//因為是最後一個TargetWord，寫入後，讀取下一行-->刷回最原本的迴圈設定
							fileRead.ReadString(strRead);
							itVV = vVstrTargetWord.begin();
							break;
						}
						else {//不是最後一個TargetWord，換下一個TargetWord比較
							continue;
						}
					}
					else {
						itVV++;
						break;//沒有符合AND，換下一個Or比較
					}
				}
			}
		}
	}
	fileWrite.WriteString(L"\n");//寫完，串到下一個資料的時候預留的空行
	fileWrite.Close();
	fileRead.Close();
}