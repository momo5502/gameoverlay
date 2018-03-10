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

		static void get_dimension(HDC hdc, LPPOINT point);

	private:

		HDC hdc = nullptr;

		GLuint texture = 0;
		GLuint program = 0;

		void* buffer = nullptr;
		bool update_pending = false;

		uint32_t width = 0;
		uint32_t height = 0;

		std::mutex mutex;

		void prepare(HDC hdc);
		void draw_internal();

		void release();

		void create(HDC hdc);
		void create(uint32_t width, uint32_t height);

		void apply_update();
	};
}
