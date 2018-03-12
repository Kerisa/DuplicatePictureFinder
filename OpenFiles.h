
#pragma once

#include <string>
#include <vector>

void ExpandDirectory(
    _In_ const std::wstring &indir,
    _Out_ std::vector<std::wstring> &resultList,
    _Out_ int *pfileNum
);
void GetSubFileList(
    std::vector<std::wstring>& inputPath,
    std::vector<std::wstring>& resultList);
//void OnDropFiles	(HDROP hDrop, std::vector<std::wstring> & resultList);
//BOOL PopFileOpenDlg (HWND hwnd, OPENFILENAME *pofn, LPTSTR filename);
//BOOL PopFileSaveDlg (HWND hwnd, OPENFILENAME *pofn, LPTSTR szSaveFile);

