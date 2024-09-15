#include <iostream>

#include "Core/ParfaitEngine.h"

int main()
{
	try
	{
		Parfait::ParfaitEngine parfaitEngine(800, 600);
		parfaitEngine.Run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << "\n";
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}