#pragma once

namespace gameoverlay
{
	class canvas
	{
	public:
		virtual ~canvas() {};

		virtual bool is_available() = 0;

		virtual uint32_t get_width() = 0;
		virtual uint32_t get_height() = 0;

		virtual bool paint(const void* buffer) = 0;
	};
}