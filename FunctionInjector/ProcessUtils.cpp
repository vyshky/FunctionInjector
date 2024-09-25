#include <intrin.h>
#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <psapi.h>
#include <string>
#include <vector>
#include <iomanip>
using namespace std;

class ProcessUtils {

public:
	ProcessUtils() {

	}
	~ProcessUtils() {

	}
	static DWORD findProcess(const WCHAR* processName) {
		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		LPPROCESSENTRY32 pe = new PROCESSENTRY32{};
		pe->dwSize = sizeof(PROCESSENTRY32);
		if (!Process32First(snapshot, pe))
		{
			cerr << "Error not find first process 'SYSTEM'" << endl;
		}
		while (Process32Next(snapshot, pe)) // Поиск нашего процесса
		{
			if (wcscmp(pe->szExeFile, processName) == 0)
			{
				wcout << "Finded process - " << processName << endl;
				break;
			}
		}
		CloseHandle(snapshot);
		return pe->th32ProcessID;
	}

	static HANDLE openProcess(DWORD processId) {
		HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS | PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, false, processId);
		if (processHandle == nullptr || processHandle == INVALID_HANDLE_VALUE) {
			throw std::runtime_error("Failed to open process: " + std::to_string(GetLastError()));
		}
		return processHandle;
	}


	static wstring getModuleFileName(HANDLE process) {
		WCHAR path[MAX_PATH];
		if (GetModuleFileNameEx(process, NULL, path, MAX_PATH) == 0) {
			throw std::runtime_error("Failed to get module file name: " + std::to_string(GetLastError()));
		}
		return wstring(path);
	}

	static LPVOID findBaseAddress(DWORD processId, HANDLE process) {
		wstring path = getModuleFileName(process);
		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
		if (snapshot == INVALID_HANDLE_VALUE) {
			cerr << "Failed to create toolhelp snapshot." << endl;
			throw std::runtime_error("Failed to create toolhelp snapshot: " + std::to_string(GetLastError()));
		}

		MODULEENTRY32 moduleEntry;
		moduleEntry.dwSize = sizeof(MODULEENTRY32);

		if (!Module32First(snapshot, &moduleEntry)) {
			CloseHandle(snapshot);
			throw std::runtime_error("Failed to get first module: " + std::to_string(GetLastError()));
		}

		do {
			if (wcscmp(moduleEntry.szExePath, &path[0]) == 0) {
				break;
			}
		} while (Module32Next(snapshot, &moduleEntry));

		CloseHandle(snapshot);
		return moduleEntry.modBaseAddr;
	}
};