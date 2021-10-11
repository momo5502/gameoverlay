#include <std_include.hpp>
#include "renderer.hpp"

/*****************************************************************************
 *
 ****************************************************************************/

void renderer::set_frame_handler(frame_handler handler)
{
	this->handler_ = std::move(handler);
}

/*****************************************************************************
 *
 ****************************************************************************/

void renderer::on_frame(canvas& canvas) const
{
	if (this->handler_) {
		this->handler_(canvas);
	}
}
