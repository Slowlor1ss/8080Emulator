#include "ConsoleWindow.h"
#include <fcntl.h>
#include <iomanip>
#include <comdef.h>
#include <iostream>

ConsoleWindow::ConsoleWindow()
{
	m_pOutStream = &std::cout;

	if (AllocConsole()) //if no console add one
	{
		// Redirect the CRT standard input, output, and error handles to the console
		FILE* pCout;
		freopen_s(&pCout, "CONIN$", "r", stdin);
		freopen_s(&pCout, "CONOUT$", "w", stdout);
		freopen_s(&pCout, "CONOUT$", "w", stderr);

		//Clear the error state for each of the C++ standard stream objects. We need to do this, as
		//attempts to access the standard streams before they refer to a valid target will cause the
		//iostream objects to enter an error state. In versions of Visual Studio after 2005, this seems
		//to always occur during startup regardless of whether anything has been read from or written to
		//the console or not.
		std::wcout.clear();
		std::cout.clear();
		std::wcerr.clear();
		std::cerr.clear();
		std::wcin.clear();
		std::cin.clear();
		std::cin.clear();

		//Set ConsoleHandle
		m_pConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

		//Disable Close-Button (closing console would cause memory leaks)
		if (HWND hwnd = GetConsoleWindow(); hwnd != nullptr)
		{
			HMENU hMenu = GetSystemMenu(hwnd, FALSE);
			if (hMenu != nullptr) DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
		}
	}
	else //Already has console
	{
		//Set ConsoleHandle
		m_pConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

		//Disable Close-Button (closing console would cause memory leaks)
		if (HWND hwnd = GetConsoleWindow(); hwnd != nullptr)
		{
			HMENU hMenu = GetSystemMenu(hwnd, FALSE);
			if (hMenu != nullptr) DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
		}
	}
}

void ConsoleWindow::Clear()
{
	std::system("cls");
}

void ConsoleWindow::Restore()
{
	if (HWND hwnd = GetConsoleWindow(); hwnd != nullptr)
	{
		ShowWindow(hwnd, SW_RESTORE); //shows window in case it minimized
		//SetForegroundWindow(hwnd); //sets it to top
	}
}
