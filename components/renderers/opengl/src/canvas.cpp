#include "std_include.hpp"
#include "canvas.hpp"

namespace gameoverlay
{
	canvas::~canvas()
	{

	}

	bool canvas::is_available()
	{
		return false;
	}

	uint32_t canvas::get_width()
	{
		return 0;
	}

	uint32_t canvas::get_height()
	{
		return 0;
	}

	bool canvas::paint(const void* buffer)
	{
		buffer;
		return false;
	}

	void canvas::draw(HDC hdc)
	{
		if (!hdc) return;

	}

	void canvas::reset()
	{
	}
}
