#include <windows.h>
//#include <tlhelp32.h>
//#include <iostream>
//#include <psapi.h>
//#include <string>
//#include <vector>
//#include <iomanip>
#include "MemoryEx.cpp" // Заменить на общий хедер

class CaveInjectorService {
	// Хранение типо вот такокго АДРЕСС->pair{ process , oldCode}
	// при закрытии инжекта закрывать возвращать oldCode
	// удалять выделенный мемори
	// закрывать process
	vector<pair<LPVOID, vector<BYTE>>> addresses{};
public:
	// Вырезает оригинальную функцию от originalAddressStart до originalAddressEnd, делее записывает в
	// конец caveFunction, после, также в конец записывает JMP originalAddressStart, создает новую память и записывает полученный код
	// в новую память процесса.
	// По адрессу originalAddressEnd, записывается JMP на новую память
	void inject(HANDLE process, const DWORD_PTR originAddressStart, const DWORD_PTR originAddressEnd, const vector<BYTE> caveFunction) {
		MemoryEx managerMem = MemoryEx();
		managerMem.isReadeble(process, originAddressStart);
		vector<BYTE> originCode = managerMem.ReadMemory(process, originAddressStart, originAddressEnd);
		vector<BYTE> combinedCode;
		combinedCode.reserve(originCode.size() + caveFunction.size());
		combinedCode.insert(combinedCode.end(), originCode.begin(), originCode.end());
		combinedCode.insert(combinedCode.end(), caveFunction.begin(), caveFunction.end());


		vector<BYTE> jmp = { 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00 };            // команда JMP
		vector<BYTE> returnAddress = pointerToByteVector((LPVOID)originAddressEnd);  // адресс прыжка на originalAddressEnd
		combinedCode.insert(combinedCode.end(), jmp.begin(), jmp.end());
		combinedCode.insert(combinedCode.end(), returnAddress.begin(), returnAddress.end());

		LPVOID newMemoryAddress = managerMem.allocateMemory(process, combinedCode.size());
		managerMem.writeCode(process, newMemoryAddress, combinedCode);


		// Очищаем код, чтобы записать прыжок на нашу новую функцию
		combinedCode.clear();
		vector<BYTE> jmpNewMemoryAddress = pointerToByteVector(newMemoryAddress);  // адресс прыжка на caveFunction
		combinedCode.insert(combinedCode.end(), jmp.begin(), jmp.end());
		combinedCode.insert(combinedCode.end(), jmpNewMemoryAddress.begin(), jmpNewMemoryAddress.end());

		managerMem.writeCode(process, (LPVOID)originAddressStart, combinedCode);         // делаем JMP на нашу функцию
		//managerMem.clearMemory(process, newMemoryAddress);
	}


	void close(HANDLE process) {
		// Хранение типо вот такокго АДРЕСС->pair{ process , oldCode}
		// при закрытии инжекта закрывать возвращать oldCode
		// удалять выделенный мемори
		// закрывать process
	}

	std::vector<BYTE> pointerToByteVector(LPVOID ptr) {
		std::vector<BYTE> byteVector;

		// Преобразуем LPVOID в 64-битное целое число (uintptr_t гарантирует 64-битный размер для указателей)
		uint64_t address = reinterpret_cast<uint64_t>(ptr);

		// 64-битный указатель = 8 байтов
		for (size_t i = 0; i < 8; ++i) {
			// Извлекаем младший байт на каждой итерации
			byteVector.push_back(static_cast<BYTE>((DWORD_PTR)(address >> (i * 8))));
		}
		return byteVector;
	}



};