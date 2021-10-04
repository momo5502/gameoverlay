#include <std_include.hpp>
#include "renderer.hpp"

void renderer::on_frame(canvas& canvas) const
{
	if (this->handler_) {
		this->handler_(canvas);
	}
}
