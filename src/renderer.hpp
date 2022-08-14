#pragma once
#include "canvas.hpp"

/*****************************************************************************
 *
 ****************************************************************************/

enum class backend_type
{
	d3d8,
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

class renderer
{
public:
	renderer() = default;

	renderer(renderer&&) = delete;
	renderer(const renderer&) = delete;
	renderer& operator=(renderer&&) = delete;
	renderer& operator=(const renderer&) = delete;

	virtual ~renderer() = default;
	virtual backend_type get_backend_type() const = 0;
	//virtual void* get_native_window() const = 0;

protected:
	void on_frame(canvas& canvas);
};

/*****************************************************************************
 *
 ****************************************************************************/

template <backend_type Type>
class typed_renderer : public renderer
{
public:
	backend_type get_backend_type() const override
	{
		return Type;
	}
};
