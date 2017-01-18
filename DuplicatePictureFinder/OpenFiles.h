
#pragma once

#include <string>
#include <vector>

void ExpandDirectory(
    _In_ const std::string &indir,
    _Out_ std::vector<std::string> &resultList,
    _Out_ int *pfileNum
);
void GetSubFileList(
    std::vector<std::string>& inputPath,
    std::vector<std::string>& resultList);
//void OnDropFiles	(HDROP hDrop, std::vector<std::string> & resultList);
//BOOL PopFileOpenDlg (HWND hwnd, OPENFILENAME *pofn, LPTSTR filename);
//BOOL PopFileSaveDlg (HWND hwnd, OPENFILENAME *pofn, LPTSTR szSaveFile);

