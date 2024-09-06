#include <iostream>

#include "Core/ParfaitEngine.h"

int main()
{
	ParfaitEngine pfEngine;

	try
	{
		pfEngine.Run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << "\n";
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}