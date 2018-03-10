#pragma once

#include "icanvas.hpp"

namespace gameoverlay
{
	class canvas : public icanvas
	{
	public:
		virtual ~canvas() override;

		virtual bool is_available() override;

		virtual uint32_t get_width() override;
		virtual uint32_t get_height() override;

		virtual bool paint(const void* buffer) override;

		void draw(HDC hdc);
		void reset();

	private:
	};
}
