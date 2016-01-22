#ifdef __linux__


#elif _WIN32

#include <Windows.h>

int __cdecl isatty(int) {

	CONSOLE_SCREEN_BUFFER_INFO sbi;
	DWORD mode;
	if (!GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &mode)) {
		return 0;
	} else {
		return 1;
	}
}

#endif
