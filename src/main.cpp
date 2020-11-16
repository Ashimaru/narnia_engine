#include "Application.h"
#include <iostream>


int main()
{
	try{
	Application app;
	app.run();
	}
	catch(...)
	{
		std::cout << "Uncaught Exception thrown";
	}

	return 0;
}
