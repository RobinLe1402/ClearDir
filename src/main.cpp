#include <string>
#include <vector>

#define NOMINMAX
#include <Windows.h>

using namespace std::string_literals;



void ShowError(const wchar_t *szMessage);

// get all the paths given to the application via parameters
std::vector<std::wstring> GetPaths();

// delete all the files and subdirectories of a directory, but not the directory itself.
bool ClearDir(const wchar_t* szPath, std::wstring &sError);



int WINAPI WinMain(
	_In_     HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_     LPSTR     szCmdLine,
	_In_     int       iCmdShow)
{
	// ignored parameters
	(void)hInstance;
	(void)hPrevInstance;
	(void)szCmdLine;
	(void)iCmdShow;



	const auto oPaths = GetPaths();
	if (oPaths.empty())
	{
		ShowError(L"No paths to delete were given.");
		return 1;
	}

	std::wstring sError;
	for (const auto &sPath : oPaths)
	{
		if (!ClearDir(sPath.c_str(), sError))
			ShowError(sError.c_str());
	}



	return 0;
}



void ShowError(const wchar_t *szMessage)
{
	MessageBoxW(NULL, szMessage, L"StartupDelete", MB_ICONERROR | MB_SYSTEMMODAL);
}

std::vector<std::wstring> GetPaths()
{
	const auto szCmdLineW = GetCommandLineW();
	int argc              = 0;
	const auto argv       = CommandLineToArgvW(szCmdLineW, &argc);

	std::vector<std::wstring> result;

	if (argc > 0)
	{
		result.reserve((size_t)argc - 1);
		for (size_t iArg = 1; iArg < argc; ++iArg)
		{
			std::wstring s = argv[iArg];

			// remove trailing slashes
			while (s.ends_with(L'\\') || s.ends_with(L'/'))
			{
				s.resize(s.length() - 1);
			}

			result.push_back(argv[iArg]);
		}
	}

	return result;
}



// get the string representation of the last error
static std::wstring LastErrorString()
{
	const DWORD errorMessageID = GetLastError();
	if (errorMessageID == ERROR_SUCCESS)
		return std::wstring(); // no error

	LPWSTR szMessage = nullptr;

	size_t size = FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorMessageID,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&szMessage,
		0,
		NULL
	);

	std::wstring result(szMessage, size);

	LocalFree(szMessage);

	return result;
}

// delete a directory (and all it's contents)
bool DeleteDir(const wchar_t* szPath, std::wstring &sError);



bool ClearDir(const wchar_t* szPath, std::wstring &sError)
{
	// check length (must be shorter than MAX_PATH)
	const size_t len = wcslen(szPath);
	if (len > MAX_PATH)
	{
		sError = L"Path \""s + szPath + L"\" was too long.";
		return false;
	}

	// check if the path exists
	const DWORD dwFileAttribs = GetFileAttributesW(szPath);
	if (dwFileAttribs == INVALID_FILE_ATTRIBUTES)
	{
		sError = L"Path \""s + szPath + L"\" wasn't found.";
		return false;
	}
	// check if the path is a directory
	else if ((dwFileAttribs & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		sError = L"\""s + szPath + L"\" is not a directory.";
		return false;
	}

	std::wstring sMask;
	sMask.reserve(MAX_PATH);
	sMask += szPath;
	sMask += L"\\*";

	WIN32_FIND_DATAW wfd = {};
	HANDLE hFind = FindFirstFileW(sMask.c_str(), &wfd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		// files were found

		std::wstring sFullPath;
		sFullPath.reserve(MAX_PATH);
		sFullPath += szPath;
		sFullPath += L'\\';
		const size_t lenWithTrailingSlash = len + 1;

		do
		{
			// skip "." and ".."
			if (wcscmp(wfd.cFileName, L".") == 0 || wcscmp(wfd.cFileName, L"..") == 0)
				continue;

			// assemble full path
			sFullPath.resize(lenWithTrailingSlash);
			sFullPath += wfd.cFileName;

			// take care of readonly elements
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
			{
				if (!SetFileAttributesW(sFullPath.c_str(),
					wfd.dwFileAttributes & ~FILE_ATTRIBUTE_READONLY)
				)
				{
					sError = L"Failed to remove \"read-only\" attribute from \""s +
						sFullPath + L"\":\n" + LastErrorString();
					return false;
				}
			}

			// directory
			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (!DeleteDir(sFullPath.c_str(), sError))
					return false;
			}

			// file
			else
			{
				if (!DeleteFileW(sFullPath.c_str()))
				{
					sError =
						L"Failed to delete file \""s + sFullPath + L"\":\n" + LastErrorString();
					return false;
				}
			}


		} while (FindNextFileW(hFind, &wfd));
		FindClose(hFind);
	}

	return true;
}

bool DeleteDir(const wchar_t* szPath, std::wstring &sError)
{
	if (!ClearDir(szPath, sError))
		return false;

	const bool result = RemoveDirectoryW(szPath);

	if (!result)
		sError = L"Failed to delete directory \""s + szPath + L"\":\n" + LastErrorString();

	return result;
}
