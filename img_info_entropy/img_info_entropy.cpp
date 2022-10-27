#include <iostream>
#include <afxdlgs.h>
#include <Windows.h>
#include <tchar.h>
#include "opencv2/opencv.hpp"
#ifdef _DEBUG
#pragma comment(lib, "opencv_world453d.lib")
#else
#pragma comment(lib, "opencv_world453.lib")
#endif
#ifndef EOF
#define EOF (-1)
#endif
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif
#ifndef EXIT_READ_ERROR_1
#define EXIT_READ_ERROR_1 1
#endif
#ifndef EXIT_READ_ERROR_2
#define EXIT_READ_ERROR_2 2
#endif
#ifndef EXIT_JPG_ERROR
#define EXIT_JPG_ERROR 3
#endif
#ifndef gapTime
#define gapTime 3000
#endif
#ifndef oriName
#define oriName "Lena.jpg"
#endif
#ifndef logoName
#define logoName "Logo.jpg"
#endif
#ifndef resName
#define resName "res.jpg"
#endif
using namespace std;
using namespace cv;


#ifdef WIN32
string GF_GetEXEPath()
{
	char FilePath[MAX_PATH];
	GetModuleFileNameA(NULL, FilePath, MAX_PATH);
	(strrchr(FilePath, '\\'))[1] = 0;
	return string(FilePath);
}
#else
string GF_GetEXEPath()
{
	int rval;
	char link_target[4096];
	char* last_slash;
	size_t result_Length;
	char* result;
	string strExeDir;
	rval = readlink("/proc/self/exe", link_target, 4096);
	if (rval < 0 || rval >= 1024)
		return "";
	link_target[rval] = 0;
	last_slash = strrchr(link_target, '/');
	if (last_slash == 0 || last_slash == link_target)
		return "";
	result_Length = last_slash - link_target;
	result = (char*)malloc(result_Length + 1);
	strncpy(result, link_target, result_Length);
	result[result_Length] = 0;
	strExeDir.append(result);
	strExeDir.append("/");
	free(result);
	return strExeDir;
}
#endif

WCHAR* CCharToLpcwstr(const char* strings)//转换宽字符
{
	int ASize = MultiByteToWideChar(CP_ACP, 0, strings, -1, NULL, 0);
	WCHAR* PwszUnicode = (wchar_t*)malloc(ASize * sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0, strings, -1, PwszUnicode, ASize);
	return PwszUnicode;
}

BOOLEAN FindFirstFileExists(LPCTSTR lpPath, BOOL dwFilter)//检查文件是否存在
{
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(lpPath, &fd);
	BOOL bFilter = (FALSE == dwFilter) ? TRUE : fd.dwFileAttributes & dwFilter;
	BOOL RetValue = ((hFind != INVALID_HANDLE_VALUE) && bFilter) ? TRUE : FALSE;
	if (RetValue == FALSE)
	{
		TCHAR SystemPath[MAX_PATH];
		GetSystemDirectory(SystemPath, MAX_PATH);
		TCHAR* _str = new TCHAR[(size_t)lstrlen(SystemPath) + (size_t)lstrlen(L"\\") + (size_t)lstrlen(lpPath) + 1];
		if (_str)
		{
			_str[0] = _T('\0');
			lstrcat(_str, SystemPath);
			lstrcat(_str, L"\\");
			lstrcat(_str, lpPath);
			WIN32_FIND_DATA _fd;
			HANDLE _hFind = FindFirstFile(_str, &_fd);
			BOOL _bFilter = (FALSE == dwFilter) ? TRUE : _fd.dwFileAttributes & dwFilter;
			BOOL _RetValue = ((_hFind != INVALID_HANDLE_VALUE) && _bFilter) ? TRUE : FALSE;
			RetValue = _RetValue;
			delete[]_str;
			FindClose(_hFind);
		}
	}
	FindClose(hFind);
	return RetValue;
}

string getPath()
{
#ifdef UNICODE
	static _TCHAR BASED_CODE szFilter[] = L"JPG Files|*.jpg|JPEG Files|*jpeg|PNG Files|*.png|BMP Files|*.bmp||";
	_TCHAR defaultExName[] = L"jpg";
	_TCHAR defaultFileName[MAX_PATH] = { 0 };
	_stprintf_s(defaultFileName, MAX_PATH, _T("%S"), (GF_GetEXEPath() + oriName).c_str());
#else
	static char BASED_CODE szFilter[] = "JPG Files|*.jpg|JPEG Files|*jpeg|PNG Files|*.png|BMP Files|*.bmp||";
	char defaultExName[] = "jpg";
	char defaultFileName[MAX_PATH] = "";
	strcpy_s(defaultFileName, (GF_GetEXEPath() + oriName).c_str(), MAX_PATH);
#endif
	CFileDialog dlg(TRUE, defaultExName, defaultFileName, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter);
	if (dlg.DoModal() == IDOK)
		return LPCSTR(CStringA(dlg.GetPathName()));
	else
		return "";
}

double Entropy(Mat img)
{
	double temp[256] = { 0 };

	/* 计算每个像素的累积值 */
	for (int m = 0; m < img.rows; ++m)//有效访问行列的方式
	{
		const uchar* t = img.ptr<uchar>(m);
		for (int n = 0; n < img.cols; ++n)
		{
			int i = t[n];
			++temp[i];
		}
	}

	/* 计算每个像素的概率 */
	for (int i = 0; i < 256; ++i)
		temp[i] /= (img.rows * img.cols);
	double result = 0;

	/* 根据定义计算图像熵 */
	for (int i = 0; i < 256; ++i)
		if (temp[i] != 0.0)
			result -= temp[i] * (log(temp[i]) / log(2.0));//result -= temp[i] * (log2(temp[i]));
	return result;
}




int main()
{
	system("chcp 936&title 基于最小信息熵准则的水印嵌入&color e&cls");
	string path3 = GF_GetEXEPath();
	string path1 = path3 + oriName, path2 = path3 + logoName;
	if (FindFirstFileExists(CCharToLpcwstr(path1.c_str()), FALSE) && FindFirstFileExists(CCharToLpcwstr(path2.c_str()), FALSE))
		cout << "自动捕获图片成功！" << endl << endl;
	else
	{
		cout << "未能自动捕获图片，请手动选择图片。" << endl << endl;
		Sleep(gapTime);
		path1 = getPath();
		if (path1 == "" || !FindFirstFileExists(CCharToLpcwstr(path1.c_str()), FALSE))
		{
			cout << "用户取消操作。" << endl << endl << endl;
			return EOF;
		}
		path2 = getPath();
		if (path2 == "" || !FindFirstFileExists(CCharToLpcwstr(path1.c_str()), FALSE))
		{
			cout << "用户取消操作。" << endl << endl << endl;
			return EOF;
		}
		if (path1.find_last_of("\\") != string::npos)//如果 path1 没有路径分隔符则使用程序所在目录
			path3 = path1.substr(0, path1.find_last_of("\\") + 1);
	}
	
	Mat img1 = imread(path1.c_str(), IMREAD_GRAYSCALE), img2 = imread(path2.c_str(), IMREAD_GRAYSCALE);
	if (img1.empty())
	{
		cout << "读取原图片失败！" << endl << endl << endl;
		return EXIT_READ_ERROR_1;
	}
	else
		cout << "原图片读取成功，高度为 " << img1.rows << "，宽度为 " << img1.cols << "。" << endl;
	if (img2.empty())
	{
		cout << "读取水印图片失败！" << endl << endl << endl;
		return EXIT_READ_ERROR_2;
	}
	else
		cout << "水印读取成功，高度为 " << img2.rows << "，宽度为 " << img2.cols << "。" << endl;

	int rows = img1.rows / img2.rows, cols = img1.cols / img2.cols, rr = 0, cc = 0;
	if (rows == 0 || cols == 0)
	{
		cout << endl << "原图片高度或宽度没能完全覆盖水印图片！" << endl << endl << endl;
		return EXIT_JPG_ERROR;
	}
	cout << endl << "水平分块：" << img2.cols << endl << "垂直分块：" << img2.rows << endl;

	double min_ent = -1;
	for (int i = 0; i < rows; ++i)
		for (int j = 0; j < cols; ++j)
		{
			Rect rect(i * img2.cols, j * img2.rows, img2.cols, img2.rows);
			Mat roi = img1(rect);
			double tmp = Entropy(roi);
			if (min_ent < 0 || tmp < min_ent)
			{
				min_ent = tmp;
				rr = i;
				cc = j;
			}
			cout << "分块[" << i << "][" << j << "]：" << tmp << endl;
		}
	Rect min_rect(rr * img2.cols, cc * img2.rows, img2.cols, img2.rows);

	img1(min_rect) &= img2;
	if (imwrite((path3 + resName).c_str(), img1))
		cout << endl << endl << "操作成功结束！" << endl << endl << endl;
	else
		cout << endl << endl << "操作没有完全成功！" << endl << endl << endl;
	return EXIT_SUCCESS;
}