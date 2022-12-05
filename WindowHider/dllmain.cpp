// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <iostream>
#include <string> 
static HMODULE Module;
static PWSTR ThreadDescription;
struct handle_data {
	unsigned long process_id;
	HWND window_handle;
	bool show;
};
BOOL CALLBACK enum_windows_callback(HWND handle, LPARAM lParam)
{
	handle_data& data = *(handle_data*)lParam;
	unsigned long process_id = 0;
	GetWindowThreadProcessId(handle, &process_id);
	if (data.process_id == process_id) {
		ShowWindow(handle, data.show ? SW_SHOW : SW_HIDE);
	}
	return TRUE;
}

void Init() {
	handle_data data;
	ZeroMemory(&data, sizeof(data));
	data.process_id = GetCurrentProcessId();
	data.window_handle = 0;
	data.show = wcscmp(ThreadDescription, L"SHOW") == 0;
	EnumWindows(enum_windows_callback, (LPARAM)&data);
	FreeLibraryAndExitThread(Module, 0);
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	Module = hModule;
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		GetThreadDescription(GetCurrentThread(), &ThreadDescription);
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Init, NULL, 0, NULL);
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

