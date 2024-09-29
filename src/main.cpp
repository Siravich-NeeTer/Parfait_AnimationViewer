#include <iostream>

#include "Core/ParfaitEngine.h"

int main()
{
	try
	{
		const int WIDTH = 800;
		const int HEIGHT = 600;
		Parfait::ParfaitEngine parfaitEngine;
		
		parfaitEngine.MakeWindow(WIDTH, HEIGHT, "Parfait Engine");

		parfaitEngine.Run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << "\n";
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}