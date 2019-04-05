#include "Interpreter.h"

extern Interpreter interpreter;

int main(int argc, char** argv)
{
	if (argc == 2)
	{
		if (interpreter.scan(argv[1]))
			interpreter.m_pTree->execute();
		else
			printf("Error: Interpreter scanning process failed\n");
	}
	else
		printf("Error: Only one argument <filename> is available\n");

	if (!interpreter.m_bExit)
	{	// If terminated not by user.
		std::cout << "Press Enter to Continue...";
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
	
	return 0;
}