#include "Application.h"
#include <iostream>


int main()
{
	try{
	Application app;

	int initResult{ 0 };
	initResult = app.init();
	if (0 != initResult)
	{
		std::cout << "Failed to initialize.";
		return initResult;
	}
	app.run();
	app.cleanUp();
	}
	catch(...)
	{
		std::cout << "Uncaught Exception thrown";
	}

	return 0;
}
