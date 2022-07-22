# WinApiHashGenerator
### This tool used to generate hashes of api functions in case if you need hide your IAT

## Usage
    WinApiHashGenerator.exe [dllname] [hash function name] [output mode] [functions file]
Parameters:
- dllname: a name or path to dll from which hash function will be imported
- hash function name: a name of your hash function implemented in your dll
- output mode: an integer in range between 0 and 4 which specifies output mode
  - 0: plain output mode, e.g.: functionHash 0xDEADBEEF
  - 1: C/C++ enum output mode, e.g: 
  ```C++
  enum FUNCTION_HASH { functionHash = 0x00C0FFEE };
  ```
  - 2: C/C++ #define directive, e.g:
  ```C++
  #define functionHash 0xABADBABE
  ```
  - 3: C++ constexpr, e.g:
  ```C++
  constexpr DWORD functionHash = 0x8BADF00D;
  ```
  - 4: C++ std::map, e.g:
  ```C++
  std::map<const char*, DWORD> nameToHash = { { "functionHash", 0xDEADFA11 } }
  ```
## Example:
### Step 1. Export your hashing function
To use this tool, you need to implement your own hashing function(or take existing one) inside a dll

DLL code:
```C++
#include <cstring>

extern "C" __declspec(dllexport) int hash(const char* name)
{
	int a = 0x3720;
	int hash = 2;

	for (int i = 0; i < strlen(name); i++)
	{
		hash *= name[i] ^ 390;
		int b = 32;
		if (i > 0)
			b = name[i - 1] % 20;
		hash -= (name[i] >> 3) + b;
	}
	
	return hash;
}
```

### Step 2. Create txt file with function names for which you need to get hashes
```
// specify your own functions, I took these just for example
NtCreateThreadEx
NtQueryProcessInformation
VirtualAlloc
GetCursorPos
GetModuleHandle
CreateFile
OpenProcess
WriteProcessMemory
ReadProcessMemory
```

### Step 3. Launch WAHG
    WinApiHashGenerator.exe myhash.dll hash 1 functions.txt
#### Program output:
```

 _    _  ___   _   _ _____
| |  | |/ _ \ | | | |  __ \
| |  | / /_\ \| |_| | |  \/
| |/\| |  _  ||  _  | | __
\  /\  / | | || | | | |_\ \
 \/  \/\_| |_/\_| |_/\____/

[+] Dll found
[*] Importing hash function
[+] Hash function found
[*] Generating list of hashes
[+] C++ code:
enum FUNCTION_HASH
{
CreateFileHash = 0x4c05a672,
GetCursorPosHash = 0xa018e268,
GetModuleHandleHash = 0x688c3449,
NtCreateThreadExHash = 0x75abcf92,
NtQueryProcessInformationHash = 0xf00b2858,
OpenProcessHash = 0x9c7a7905,
ReadProcessMemoryHash = 0x78b5634c,
VirtualAllocHash = 0x41aa6cab,
WriteProcessMemoryHash = 0xa6a9ed5c,
};
```
