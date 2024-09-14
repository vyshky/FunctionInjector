#include <windows.h>
//#include <tlhelp32.h>
#include <iostream>
//#include <psapi.h>
#include <string>
#include <vector>
#include <iomanip>
using namespace std;

class MemoryEx {
public:
	LPVOID allocateMemory(const HANDLE process, const SIZE_T sizePage) {
		LPVOID newAddress = VirtualAllocEx(process, NULL, sizePage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		if (newAddress == nullptr) {
			CloseHandle(process);
			throw std::runtime_error("Failed to write to target process memory: " + std::to_string(GetLastError()));
		}
		return newAddress;
	}

	vector<BYTE> ReadMemory(const HANDLE process, const DWORD_PTR start, const DWORD_PTR end) {
		vector<BYTE> buffer(SIZE_T(end - start));
		SIZE_T bytesRead;
		if (!ReadProcessMemory(process, (LPCVOID)start, buffer.data(), buffer.size(), &bytesRead)) {
			throw std::runtime_error("Error read memory: " + std::to_string(GetLastError()));
		}
		return buffer;
	}

	void isReadeble(HANDLE process, DWORD_PTR address) {
		MEMORY_BASIC_INFORMATION memoryBasicInformation;
		SIZE_T result = VirtualQueryEx(process, (LPVOID)address, &memoryBasicInformation, sizeof(memoryBasicInformation));

		if (result == 0) {
			throw std::runtime_error("Error call VirtualQueryEx: " + std::to_string(GetLastError()));
		}

		if (!(memoryBasicInformation.Protect & PAGE_READONLY || memoryBasicInformation.Protect & PAGE_READWRITE ||
			memoryBasicInformation.Protect & PAGE_EXECUTE_READ || memoryBasicInformation.Protect & PAGE_EXECUTE_READWRITE)) {
			throw std::runtime_error("Memory at address is not readable: " + std::to_string(GetLastError()));
		}
	}

	void writeCode(HANDLE process, LPVOID address ,vector<BYTE> combinedCode) {
		if (!WriteProcessMemory(process, address, combinedCode.data(), combinedCode.size(), NULL)) {
			VirtualFreeEx(process, address, 0, MEM_RELEASE);
			throw std::runtime_error("Failed to write to target process memory: " + std::to_string(GetLastError()));
		}
	}

	BOOL clearMemory(HANDLE process, LPVOID address) {
		return VirtualFreeEx(process, address, 0, MEM_RELEASE);
	}

	BOOL closeProcess(HANDLE process) {
		return CloseHandle(process);
	}

};