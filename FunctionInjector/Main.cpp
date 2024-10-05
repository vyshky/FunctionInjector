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
#include "ProcessUtils.cpp"
using namespace std;

static LPVOID vectorToAddress(std::vector<BYTE>& bytes);
static bool  ReadBytes(HANDLE hProcess, LPVOID address, BYTE* buffer, SIZE_T sizeByte);
static bool WriteBytes(HANDLE hProcess, LPVOID address, BYTE* buffer, SIZE_T sizeByte);

//0x48, 0x81, 0xC1, 0x20, 0x01, 0x00, 0x00, - add rcx, 00000120
//0x48, 0x89, 0x0D, 0x18, 0x00, 0x00, 0x00, - mov[1CCDB300039], rcx
//0x48, 0x81, 0xE9, 0x20, 0x01, 0x00, 0x00 - sub rcx, 00000120



int main() {
	vector<BYTE> vec = {
		0x48, 0x81, 0xC1, 0x20, 0x01, 0x00, 0x00,
		0x48, 0x89, 0x0D, 0x15, 0x00, 0x00, 0x00,
		0x48, 0x81, 0xE9, 0x20, 0x01, 0x00, 0x00
	};
	DWORD id = ProcessUtils::findProcess(L"SkyrimSE.exe");
	HANDLE process = ProcessUtils::openProcess(id);
	LPVOID baseAddres = ProcessUtils::findBaseAddress(id, process);
	DWORD_PTR injectOffsetStart = (DWORD_PTR)baseAddres + 0xA87BE0;
	DWORD_PTR injectOffsetEnd = (DWORD_PTR)baseAddres + 0xA87BFF;
	SIZE_T sizeCaveFunction = injectOffsetEnd - injectOffsetStart;
	CaveInjectorService injector = CaveInjectorService();
	LPVOID endCaveFunction = injector.inject(process, injectOffsetStart, injectOffsetEnd, vec);


	// Вытаскиваем адресс координат
	MemoryEx managerMem = MemoryEx();
	vector<BYTE> addressCoordsArray = managerMem.ReadMemory(process, (DWORD_PTR)endCaveFunction, (DWORD_PTR)endCaveFunction + 8);

	BYTE zeroArray[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	if (addressCoordsArray.size() == 8) {
		int iterator = 0;
		// Сравниваем данные с помощью memcmp
		while (memcmp(addressCoordsArray.data(), zeroArray, 8) == 0 && iterator < 5) {
			addressCoordsArray = managerMem.ReadMemory(process, (DWORD_PTR)endCaveFunction, (DWORD_PTR)endCaveFunction + 8);
			Sleep(1000);
			iterator++;
		}
		for (BYTE byte : addressCoordsArray) {
			// Вывод в hex формате, минимум 2 символа, с ведущими нулями
			std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
		}
	}

	// Сохраняем в переменную
	LPVOID  addressCoodrsXYZ = vectorToAddress(addressCoordsArray);
	LPVOID zCord = (LPVOID)((BYTE*)addressCoodrsXYZ + 8);
	BYTE buffer[4]{ };
	ReadBytes(process, zCord, buffer, 4);

	//////////////////////////////////////////////////////// Преобразовать байты в целое число (Big Endian)
	//unsigned int value = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];  // Старший байт первый

	//std::cout << "Original value (Big Endian): " << value << std::endl;
	std::cout << "Exit button Q" << std::endl;
	bool playerUp = false;
	bool playerDown = false;
	bool flyON = false;
	Sleep(1000);
	for (unsigned int i = buffer[2]; true;) {
		if (i > 255) {
			i = 0;
		}

		if (GetAsyncKeyState(VK_DELETE) & 0x8000) {
			flyON = !flyON;
			ReadBytes(process, zCord, buffer, 4);
			i = buffer[2];
			Sleep(500);
		}

		if (flyON) {

			if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
				playerUp = true;
			}
			else {
				playerUp = false;
			}

			if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
				playerDown = true;
			}
			else {
				playerDown = false;
			}

			if (playerUp) {
				buffer[2] = ++i;
				if (i > 255) {
					i = 0;
					buffer[2] = 0;
					buffer[3] += 1;
				}
				WriteBytes(process, zCord, buffer, 4);
				Sleep(100);
			}
			else if (playerDown) {
				buffer[2] = --i;
				if (i == 0) {
					i = 255;
					buffer[2] = 255.0f;
					buffer[3] -= 1;
				}
				WriteBytes(process, zCord, buffer, 4);
				Sleep(100);
			}
			else
			{
				WriteBytes(process, zCord, buffer, 4);
			}
		}
		// Проверяем, нажата ли клавиша 'Q'
		if (GetAsyncKeyState('Q') & 0x8000) {
			std::cout << "Programm Closed" << std::endl;
			break;  // Выход из цикла
		}
	}

	injector.close(process);
	return 0;
}

LPVOID vectorToAddress(std::vector<BYTE>& bytes) {
	// Проверяем, что длина вектора соответствует размеру указателя (8 байт для x64)
	if (bytes.size() != sizeof(uintptr_t)) {
		std::cerr << "Ошибка: длина вектора не соответствует размеру указателя." << std::endl;
		return nullptr;
	}

	// Переменная для хранения адреса
	uintptr_t address = 0;

	// Собираем байты в обратном порядке (меняем little-endian на big-endian)
	for (size_t i = 0; i < bytes.size(); ++i) {
		// Сдвигаем предыдущие байты влево и добавляем текущий байт (в обратном порядке)
		address |= static_cast<uintptr_t>(bytes[i]) << (i * 8);
	}

	// Возвращаем адрес как LPVOID
	return reinterpret_cast<LPVOID>(address);
}

bool WriteBytes(HANDLE hProcess, LPVOID address, BYTE* buffer, SIZE_T sizeByte) {
	return WriteProcessMemory(hProcess, address, buffer, sizeByte, NULL);
}

bool ReadBytes(HANDLE hProcess, LPVOID address, BYTE* buffer, SIZE_T sizeByte) {
	return ReadProcessMemory(hProcess, address, buffer, sizeByte, NULL);
}