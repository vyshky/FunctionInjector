#include <intrin.h>
#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <psapi.h>
#include <string>
#include <vector>
#include <iomanip>
#include "CaveInjectorService.cpp"
using namespace std;

DWORD findProcess(const WCHAR* processName) {
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

HANDLE openProcess(DWORD processId) {
	HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS | PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, false, processId);
	if (processHandle == nullptr || processHandle == INVALID_HANDLE_VALUE) {
		throw std::runtime_error("Failed to open process: " + std::to_string(GetLastError()));
	}
	return processHandle;
}


void caveFunction() {
	unsigned long long rcx_val = 10;
	unsigned long long rbx_val = 20;
	unsigned long long result;

	// В Microsoft компиляторе используем Intrinsics для работы с регистрами
	__int64 rcx = rcx_val;  // Загружаем значение в регистр RCX
	__int64 rbx = rbx_val;  // Загружаем значение в регистр RBX

	__int64 sum = rcx + rbx;  // Складываем значения RCX и RBX
	result = sum;

	std::cout << "Result: " << result << std::endl;
}
int End() {
	return 0;
}

int main() {
	vector<BYTE> vec = {  };
	DWORD id = findProcess(L"SkyrimSE.exe");
	HANDLE process = openProcess(id);
	CaveInjectorService injector = CaveInjectorService();
	injector.inject(process, 0x7FF6607C7BE0, 0x7FF6607C7BFF, vec);
	Sleep(5000);
	injector.close(process);
	return 0;
}