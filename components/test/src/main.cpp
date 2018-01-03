#include "std_include.hpp"

#include <literally/library.hpp>

int main()
{
	printf("Loading overlay...\n");
	literally::dynlib::load("overlay.dll");

	_getch();
	return 0;
}
