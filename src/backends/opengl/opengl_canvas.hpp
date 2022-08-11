#pragma once
#include <GL/glew.h>

#include "../../canvas.hpp"

namespace gameoverlay::opengl
{
	class canvas : public ::canvas
	{
	public:
		canvas(const uint32_t width, const uint32_t height);
		~canvas() override;

		dimensions get_dimensions() const override;
		void paint(const void* image) override;

		void draw() const;

	private:
		const uint32_t width_{};
		const uint32_t height_{};

		GLuint texture_{};
		GLuint program_{};
	};
}
