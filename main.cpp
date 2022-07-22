// this tool used to generate hash of api functions in case if you need hide your IAT
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <Windows.h>

#define IN_RANGE(x, a, b) ((x) >= (a) && (x) <= (b)) 

typedef int(__cdecl* hashFunction_t)(const char*);

enum OUTPUT_MODE
{
	OUTPUT_PLAIN = 0,
	OUTPUT_ENUM,
	OUTPUT_DEFINE,
	OUTPUT_CONSTEXPR,
	OUTPUT_MAP,
	
	OUTPUT_MODE_COUNT
};

bool isNumber(std::string string);
void printResults(OUTPUT_MODE mode, std::map<std::string, DWORD> nameToHash);

int main(int argc, char** argv)
{
	std::cout << R"(
 _    _   ___  ______  _____ 
| |  | | / _ \ | ___ \|  __ \
| |  | |/ /_\ \| |_/ /| |  \/
| |/\| ||  _  ||  __/ | | __ 
\  /\  /| | | || |    | |_\ \
 \/  \/ \_| |_/\_|     \____/
		)" << std::endl;


	if (argc < 5)
	{
		std::cout << "[*] Put your dll with exported hash function in the same folder\n";
		std::cout << "[*] Usage: " << argv[0] << " [dllname] [hash function name] [output mode] [function names file(one per line)]\n";
		std::cout << "[*] Example: " << argv[0] << " myhash.dll hashFunction 0 functions.txt\n";
		std::cout << "[*] Output modes:\n";
		std::cout << std::left << std::setw(20) << "[*] name" << "| mode\n";
		std::cout << std::left << std::setw(20) << "[*] plain" << "| 0\n";
		std::cout << std::left << std::setw(20) << "[*] enum" << "| 1\n";
		std::cout << std::left << std::setw(20) << "[*] define" << "| 2\n";
		std::cout << std::left << std::setw(20) << "[*] constexpr" << "| 3\n";
		std::cout << std::left << std::setw(20) << "[*] std::map" << "| 4\n";
		std::cout << "[*] Dll code should be:\n";
		std::cout << "[*] extern \"C\" __declspec(dllexport) int hashFunction(const char* name) { /*your hashing code*/ }\n";
		std::cout << "[*] where hashFunction is your function name\n";
		return 0;
	}

	std::vector<std::string> names;
	std::ifstream file(argv[4]);
	std::string line;
	while (std::getline(file, line))
		names.push_back(line);

	if (!isNumber(std::string(argv[3])))
	{
		std::cout << "[*] Output mode parameter should be a number\n";
		return 0;
	}

	int outputMode = atoi(argv[3]);
	if (!IN_RANGE(outputMode, 0, OUTPUT_MODE_COUNT - 1))
	{
		std::cout << "[*] Output mode parameter should be between 0 and 2\n";
		return 0;
	}

	HMODULE dllHandle = LoadLibraryA(argv[1]);
	if (!dllHandle)
	{
		std::cout << "[!] Dll not found\n";
		return 0;
	}

	std::cout << "[+] Dll found\n";
	std::cout << "[*] Importing hash function\n";

	hashFunction_t hashFunction = (hashFunction_t)GetProcAddress(dllHandle, argv[2]);
	if (!hashFunction)
	{
		std::cout << "[!] Specified hash function <" << argv[2] << "> not found in " << argv[1] << '\n';
		return 0;
	}

	std::cout << "[+] Hash function found\n";
	std::cout << "[*] Generating list of hashes\n";

	std::map<std::string, DWORD> nameToHash;
	for (std::string& functionName : names)
		nameToHash[functionName] = hashFunction(functionName.c_str());
	
	if (outputMode != OUTPUT_PLAIN)
		std::cout << "[+] C++ code:\n";
	printResults((OUTPUT_MODE)outputMode, nameToHash);
	
	return 0;
}

bool isNumber(std::string string)
{
	bool isFirst = true;
	for (char& c : string)
	{
		if (isFirst)
		{
			isFirst = false;
			if (c == '-')
				continue;
		}
		if (!isdigit(c))
			return false;
	}

	return true;
}

void printResults(OUTPUT_MODE mode, std::map<std::string, DWORD> nameToHash)
{
	switch (mode)
	{
		case OUTPUT_PLAIN:
		{
			std::cout << std::left << std::setw(60) << "Function name" << "Hash\n";
			std::cout << "-------------------------------------------------------------------------\n";
			for (std::pair<std::string, DWORD> pair : nameToHash)
				std::cout << std::left << std::setw(60) << pair.first << std::hex << "0x" << pair.second << '\n';
			break;
		}

		case OUTPUT_ENUM:
		{
			std::cout << "enum FUNCTION_HASH\n{\n";

			for (std::pair<std::string, DWORD> pair : nameToHash)
				std::cout << pair.first << "Hash" << " = " << std::hex << "0x" << pair.second << ",\n";
			
			std::cout << "}\n";
			break;
		}

		case OUTPUT_DEFINE:
		{
			for (std::pair<std::string, DWORD> pair : nameToHash)
				std::cout << "#define " << pair.first << "Hash " << std::hex << "0x" << pair.second << '\n';
			break;
		}
		
		case OUTPUT_CONSTEXPR:
		{
			for (std::pair<std::string, DWORD> pair : nameToHash)
				std::cout << "constexpr DWORD " << pair.first << "Hash = " << std::hex << "0x" << pair.second << ";\n";
			break;
		}

		case OUTPUT_MAP:
		{
			std::cout << "std::map<const char*, DWORD> nameToHash = { ";
			for (std::pair<std::string, DWORD> pair : nameToHash)
				std::cout << "{ \"" << pair.first << "\", " << std::hex << "0x" << pair.second << " }, ";
			std::cout << " };\n";
			break;
		}
	}
}
