
//#include <commctrl.h>
#include <windows.h>
#include "OpenFiles.h"


void ExpandDirectory(
    _In_ const std::wstring &indir,
    _Out_ std::vector<std::wstring> &resultList,
    _Out_ int *pfileNum
)
{
    HANDLE			hFindFile;
    WIN32_FIND_DATA FindData;
    int				cnt = 0;

    std::wstring slash(indir), wildcard(indir), find;
    if (indir.back() != '\\')
    {
        slash += L"\\";
        wildcard += L"\\*.*";
    }
    else
        wildcard += L"*.*";

    if (INVALID_HANDLE_VALUE != (hFindFile = FindFirstFile(wildcard.c_str(), &FindData)))
    {
        do
        {
            find.assign(slash);
            find += FindData.cFileName;
            if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (wcscmp(FindData.cFileName, L".") &&
                    wcscmp(FindData.cFileName, L".."))
                    ExpandDirectory(find, resultList, pfileNum);
            }
            else
            {
                resultList.push_back(find);
                ++(*pfileNum);
            }
        } while (FindNextFile(hFindFile, &FindData));
        FindClose(hFindFile);
    }
}

void GetSubFileList(std::vector<std::wstring> &inputPath, std::vector<std::wstring>& resultList)
{
    resultList.clear();
    for (int i = 0; i < inputPath.size(); ++i)
    {
        DWORD attr = GetFileAttributes(inputPath[i].c_str());
        int fileNum = 0;
        if (attr == INVALID_FILE_ATTRIBUTES)
            continue;
        else if (attr == FILE_ATTRIBUTE_DIRECTORY)
            ExpandDirectory(inputPath[i], resultList, &fileNum);
        else
            resultList.push_back(std::wstring(inputPath[i]));
    }
}

void OnDropFiles(HDROP hDrop, std::vector<std::wstring> & inputPath)
{
	TCHAR szBuffer[MAX_PATH];
	int iNum = DragQueryFile(hDrop, 0xffffffff, NULL, 0);
    for (int i = 0; i < iNum; ++i)
    {
        DragQueryFile(hDrop, i, szBuffer, _countof(szBuffer));
        inputPath.push_back(std::wstring(szBuffer));
    }
    DragFinish(hDrop);    
	return;
}

BOOL PopFileOpenDlg (HWND hwnd, OPENFILENAME * pofn, LPTSTR filename)
{
	static TCHAR		szFilter [] = TEXT("所有文件 (*.*)\0*.*\0\0"),
						szTitle	 [] = TEXT("选择要分析的文件");

	RtlZeroMemory(pofn, sizeof(OPENFILENAME));
	pofn->lStructSize       = sizeof(OPENFILENAME);
	pofn->hwndOwner         = hwnd;
	pofn->lpstrFilter       = szFilter;
	pofn->lpstrFile         = filename;
	pofn->lpstrFileTitle    = NULL;
	pofn->nMaxFileTitle     = 0;
	pofn->nMaxFile          = MAX_PATH;
	pofn->lpstrTitle        = szTitle;
	pofn->Flags             = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;// | OFN_ALLOWMULTISELECT;
    
	return GetOpenFileName(pofn);
}

BOOL PopFileSaveDlg (HWND hwnd, OPENFILENAME * pofn, LPTSTR szSaveFile)
{
	static TCHAR		szFilter [] = TEXT("文本文件 (*.txt)\0*.txt\0所有文件 (*.*)\0*.*\0\0"),
						szTitle	 [] = TEXT("保存HASH运算结果");

	RtlZeroMemory(pofn, sizeof(OPENFILENAME));
	szSaveFile[0]       = 0;
	pofn->lStructSize   = sizeof(OPENFILENAME);
	pofn->hwndOwner     = hwnd;
	pofn->lpstrFilter   = szFilter;
	pofn->lpstrFile     = szSaveFile;
	pofn->nMaxFile      = MAX_PATH;
	pofn->lpstrTitle    = szTitle;
	pofn->lpstrDefExt   = TEXT("txt");
	pofn->Flags         = OFN_OVERWRITEPROMPT;

	return GetOpenFileName(pofn);
}