#include "Interpreter.h"
#include "RegularScope.h"
#include "Console.h"

int main(int argc, char** argv)
{
	extern Interpreter interpreter;
	if (argc == 2)
	{
		if (interpreter.scan(argv[1], nullptr, -1))
			interpreter.m_pTree->execute();
	}
	else if (argc < 2)
		PrintFatalError(nullptr, -1, "Argument <filename> is not specified");
	else
		PrintFatalError(nullptr, -1, "Only argument <filename> is available");

	if (interpreter.m_bError)
	{	// If terminated not by user.
		std::cout << "Press Enter to Continue...";
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
	
	std::vector<CommonScope::m_rofile_type> &rofiles = CommonScope::rofiles();
	if (!rofiles.empty())
	{	// Only global scope can access this code.
		std::fstream* fptr = nullptr;
		for (auto it = rofiles.begin(); it != rofiles.end(); ++it)
		{
			fptr = std::get<std::fstream*>(*it);
			if (fptr->is_open())
				fptr->close();
			delete fptr;
		}
		rofiles.clear();
	}

	// Destroy tree.
	interpreter.m_pTree->destroy();
	return 0;
}