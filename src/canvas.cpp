#include <std_include.hpp>
#include "canvas.hpp"

/*****************************************************************************
 *
 ****************************************************************************/

uint32_t canvas::get_width() const
{
	return this->get_dimensions().width;
}

/*****************************************************************************
 *
 ****************************************************************************/

uint32_t canvas::get_height() const
{
	return this->get_dimensions().height;
}

/*****************************************************************************
 *
 ****************************************************************************/

fixed_canvas::fixed_canvas(const uint32_t width, const uint32_t height):
	width_(width), height_(height)
{
}

/*****************************************************************************
 *
 ****************************************************************************/

dimensions fixed_canvas::get_dimensions() const
{
	dimensions dimensions{};
	dimensions.width = this->width_;
	dimensions.height = this->height_;
	return dimensions;
}
