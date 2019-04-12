#include "Interpreter.h"
#include "Console.h"

//extern RegularScope globalScope;
extern Interpreter interpreter;

int main(int argc, char** argv)
{
	if (argc == 2)
	{
		if (interpreter.scan(argv[1], nullptr, -1))
			interpreter.m_pTree->execute();
	}
	else if (argc < 2)
		PrintFatalError(nullptr, -1, "Argument <filename> is not specified");
	else
		PrintFatalError(nullptr, -1, "Only argument <filename> is available");

	if (!interpreter.m_bExit)
	{	// If terminated not by user.
		std::cout << "Press Enter to Continue...";
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
	
	std::vector<CommonScope::m_rofile_type> &rofiles = CommonScope::rofiles();
	if (!rofiles.empty())
	{	// Only global scope can access this code
		for (auto it = rofiles.begin(); it != rofiles.end(); ++it)
		{
			if ((*it).first->is_open())
				(*it).first->close();
			delete (*it).first;
		}
		rofiles.clear();
	}

	// Destroy tree.
	interpreter.m_pTree->destroy();
	return 0;
}