#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <iomanip>
#include <Shlwapi.h>
#include <comdef.h>
#include <string> 
#pragma comment( lib, "shlwapi.lib")
#define print(format, ...) printf (format, __VA_ARGS__)
using namespace std;
const char dllname[] = "WindowHider.dll";

inline int panic() {
	// std::system("pause");
	return EXIT_FAILURE;
}

DWORD GetProcId(const char* pn, unsigned short int fi = 0b1101)
{
	DWORD procId = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 pE;
		pE.dwSize = sizeof(pE);

		if (Process32First(hSnap, &pE))
		{
			if (!pE.th32ProcessID)
				Process32Next(hSnap, &pE);
			do
			{
				if (fi == 0b10100111001)
					std::cout << pE.szExeFile << u8"\x9\x9\x9" << pE.th32ProcessID << std::endl;

				_bstr_t bExeName(pE.szExeFile);
				if (!_stricmp(bExeName, pn))
				{
					procId = pE.th32ProcessID;
					print("Process : 0x%lX\n", pE);
					break;
				}
			} while (Process32Next(hSnap, &pE));
		}
	}
	CloseHandle(hSnap);
	return procId;
}


BOOL InjectDLL(DWORD procID, const char* dllPath, bool show)
{
	BOOL WPM = 0;

	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, procID);
	if (hProc == INVALID_HANDLE_VALUE)
	{
		return -1;
	}
	void* loc = VirtualAllocEx(hProc, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (loc == 0)
		return -1;
	WPM = WriteProcessMemory(hProc, loc, dllPath, strlen(dllPath) + 1, 0);
	if (!WPM)
	{
		CloseHandle(hProc);
		return -1;
	}
	print("DLL Injected Succesfully 0x%lX\n", WPM);
	HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, loc, CREATE_SUSPENDED, 0);
	if (hThread == 0)
	{
		CloseHandle(hProc);
		return -1;
	}
	SetThreadDescription(hThread, show ? L"SHOW" : L"HIDE");
	ResumeThread(hThread);
	print("Thread Created Succesfully %p\n", hThread);
	print("Waiting for remote thread to return\n");
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hProc);
	VirtualFree(loc, 0, MEM_RELEASE);
	CloseHandle(hThread);
	return 0;
}

wstring ExePath() {
	WCHAR buffer[MAX_PATH] = { 0 };
	GetModuleFileNameW(NULL, buffer, MAX_PATH);
	wstring::size_type pos = wstring(buffer).find_last_of(L"\\/");
	return wstring(buffer).substr(0, pos);
}
inline void printHelp() {
	print("CLI usage: \nInjector [show|hide] [<PID>|<ImageName>]\n");
}
inline long GetPid(string pidOrProcessName) {
	try {
		return stoi(pidOrProcessName);
	}
	catch (exception ex) {
		return GetProcId(pidOrProcessName.c_str());
	}
}
int wmain(int argc, wchar_t* argv[])
{
	SetCurrentDirectoryW(ExePath().c_str());
	char dllpath[MAX_PATH] = { 0 };
	GetFullPathNameA(dllname, MAX_PATH, dllpath, nullptr);
	print("Dll path to inject is %s \n", dllpath);
	if (!PathFileExistsA(dllpath))
	{
		print("DLL File does not exist!");
		return panic();
	}
	DWORD procId = 0;
	bool show = false;
	if (argc > 1) {
		if (argc < 3) {
			printHelp();
			return panic();
		}
		if (wcscmp(argv[1], L"show") == 0) {
			show = true;
		}
		else if (wcscmp(argv[1], L"hide") == 0) {
			show = false;
		}
		else {
			printHelp();
			return panic();
		}
		wstring pidOrName(argv[2]);
		procId = GetPid(string(pidOrName.begin(), pidOrName.end()));
	}
	else {
		print("process name (e.g. chrome.exe ) or pid:");
		string pname;
		cin >> pname;
		procId = GetPid(pname);
		system("cls");
		if (procId == NULL)
		{
			print("Process Not found (0x%lX)\n", GetLastError());
			print("Here is a list of available process \n");
			Sleep(2000);
			system("cls");
			GetProcId("skinjbir", 0b10100111001);
			return panic();
		}
		print("Show(s) or hide(h) window?\n");
		char response;
		cin >> response;
		if (response == 's') {
			show = true;
		}
		else if (response == 'h') {
			show = false;
		}
		else {
			print("Invalid input\n");
			return panic();
		}
		system("cls");
	}
	print("Target proccess id is %ld\n", procId);
	if (InjectDLL(procId, dllpath, show) != 0) {
		print("Injection failed! error code: %d\n", GetLastError());
		print("Try running the process again as administrator.\n");
		return panic();
	}
	print("Done\n");
	// system("pause");
	return EXIT_SUCCESS;
}

