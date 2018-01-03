#pragma once

namespace gameoverlay
{
	class icanvas
	{
	public:
		virtual ~icanvas() {};

		virtual bool is_available() = 0;

		virtual uint32_t get_width() = 0;
		virtual uint32_t get_height() = 0;

		virtual bool paint(const void* buffer) = 0;
	};
}