#include <windows.h>
#include <vector>
using namespace std;

class CaveMemoryCleanModel {
public:
	HANDLE process{};
	LPVOID caveAddress{};
	LPVOID injectedAddress{};
	vector<BYTE> data{};
};