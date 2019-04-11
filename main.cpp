#include "Interpreter.h"

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
		printf("Error: Argument <filename> is not specified\n");
	else
		printf("Error: Only one argument <filename> is available\n");

	if (!interpreter.m_bExit)
	{	// If terminated not by user.
		std::cout << "Press Enter to Continue...";
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
	
	// Destroy tree.
	interpreter.m_pTree->destroy();
	return 0;
}