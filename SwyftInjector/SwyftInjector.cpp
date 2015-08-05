// SwyftInjector.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <Windows.h>

HWND ChromeMainWindow = NULL;
HWND InjectedWindow = NULL;

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lparam)
{
	//Get the window's title
	int length = GetWindowTextLength(hwnd) + 1;
	char* buffer = new char[length];
	GetWindowTextA(hwnd, buffer, length);

	//Store the HWND if it's chrome's main window
	if (strstr(buffer, "Google Chrome") > 0)
		ChromeMainWindow = hwnd;

	return TRUE;
}

int _tmain(int argc, _TCHAR* argv[])
{
	//Optional parameter for hiding console

	if (argc > 1)
		if (!wcscmp(argv[1], L"-nogui"))
		{
			HWND hWnd = GetForegroundWindow();
			ShowWindow(hWnd, SW_HIDE);
		}

	DWORD ProcessID = 0;
	HANDLE ProcessHandle = 0;
	wchar_t* DLLPath;

	while (true)
	{
		//Keep looking for chrome's main window
		while (!ChromeMainWindow || InjectedWindow == ChromeMainWindow)
		{
			EnumWindows(EnumWindowsProc, (LPARAM)0);
			Sleep(1000);
		}
		printf("Got Google Chrome's main window HWND: 0x%X.\n", ChromeMainWindow);

		//Get Chrome's main process id
		GetWindowThreadProcessId(ChromeMainWindow, &ProcessID);
		printf("Got the process id: 0x%X.\n", ProcessID);

		//Get Chrome's main process handle
		ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, false, ProcessID);
		printf("Got the process handle: 0x%X.\n", ProcessHandle);

		//Get the DLL's path
		DLLPath = argv[0];
		DLLPath[lstrlenW(DLLPath) - 17] = 0;
		wsprintf(DLLPath, L"%s%s", DLLPath, L"SwyftBow.dll");
		wprintf(L"DLL Path: %s\n", DLLPath);

		//Allocate space for the DLL Path on Chrome's process
		LPVOID pathAddress = VirtualAllocEx(ProcessHandle, NULL, lstrlenW(DLLPath) * 2, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		printf("Allocated path address: 0x%x\n", pathAddress);

		//Write the path to the allocated space
		bool success = WriteProcessMemory(ProcessHandle, pathAddress, DLLPath, lstrlenW(DLLPath) * 2, NULL);
		printf("Writing path result: %s\n", (success == true ? "Success" : "Fail"));

		//Get the address for LoadLibraryW (support for unicode paths)
		HMODULE DLLHandle = GetModuleHandle(L"kernel32.dll");
		DWORD LoadLibraryAddress = (DWORD)GetProcAddress(DLLHandle, "LoadLibraryW");
		printf("LoadLibraryW address: 0x%x\n", LoadLibraryAddress);

		//Call LoadLibrary with the DLL path as a parameter
		HANDLE ThreadHandle = CreateRemoteThread(ProcessHandle, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibraryAddress, pathAddress, NULL, NULL);
		printf("Created LoadLibraryW Thread: %x\n", ThreadHandle);

		printf("-----------------------------------------------------------------------\n", ThreadHandle);

		InjectedWindow = ChromeMainWindow;
	}

	return 0;
}

