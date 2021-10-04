#pragma once
#include "canvas.hpp"

#include <functional>

/*****************************************************************************
 *
 ****************************************************************************/

enum class backend_type {
	d3d9,
	d3d10,
	d3d11,
	d3d12,
	opengl,
	vulkan,
};

/*****************************************************************************
 *
 ****************************************************************************/

class renderer {
public:
	using frame_handler = std::function<void(canvas&)>;

	virtual ~renderer() = default;
	virtual backend_type get_backend_type() const = 0;

	void set_frame_handler(frame_handler handler);

protected:
	renderer() = default;
	void on_frame(canvas& canvas) const;

private:
	frame_handler handler_{};
};
