#pragma once
#include <ostream>

class ConsoleWindow
{
public:
	ConsoleWindow();
	~ConsoleWindow() = default;

	static void Clear();
	static void Restore();

private:
	std::ostream* m_pOutStream{};

	void* m_pConsoleHandle{ nullptr };

};

