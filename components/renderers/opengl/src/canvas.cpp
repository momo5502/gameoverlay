#include "std_include.hpp"
#include "canvas.hpp"

namespace gameoverlay
{
	canvas::~canvas()
	{
		this->release();
	}

	bool canvas::is_available()
	{
		return this->texture != NULL;
	}

	uint32_t canvas::get_width()
	{
		if (!this->is_available()) return 0;

		int w, miplevel = 0;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_WIDTH, &w);

		return uint32_t(w);
	}

	uint32_t canvas::get_height()
	{
		if (!this->is_available()) return 0;

		int h, miplevel = 0;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_HEIGHT, &h);

		return uint32_t(h);
	}

	bool canvas::paint(const void* _buffer)
	{
		std::lock_guard<std::mutex> _(this->mutex);
		if (!this->is_available() || !this->buffer) return false;

		std::memmove(this->buffer, _buffer, this->get_width() * this->get_height() * 4);
		this->update_pending = true;

		return true;
	}

	void canvas::apply_update()
	{
		std::lock_guard<std::mutex> _(this->mutex);
		if (!this->update_pending || !this->is_available()) return;
		this->update_pending = false;

		GLint texture2d;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture2d);
		glBindTexture(GL_TEXTURE_2D, this->texture);

		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, this->get_width(), this->get_height(), GL_RGBA, GL_UNSIGNED_BYTE, this->buffer);

		glBindTexture(GL_TEXTURE_2D, texture2d);
	}

	void canvas::create(HDC _hdc)
	{
		this->hdc = _hdc;

		POINT dim;
		canvas::get_dimension(this->hdc, &dim);

		this->create(dim.x, dim.y);
	}

	void canvas::create(uint32_t width, uint32_t height)
	{
		GLint texture2d;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture2d);

		int alignment = 0;
		glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glGenTextures(1, &this->texture);
		glBindTexture(GL_TEXTURE_2D, this->texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		this->buffer = new char[width * height * 4];
		ZeroMemory(this->buffer, width * height * 4);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
		glBindTexture(GL_TEXTURE_2D, texture2d);

		glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);

		// Create shaders
		GLint v_shader = glCreateShader(GL_VERTEX_SHADER);
		GLint f_shader = glCreateShader(GL_FRAGMENT_SHADER);

		GLchar* v_shader_src =
			"void main(void)"
			"{"
			"	gl_TexCoord[0] = gl_MultiTexCoord0;"
			"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
			"}";

		GLchar* f_shader_src =
			"uniform sampler2D tex_sampler;"
			"void main(void)"
			"{"
			"	gl_FragColor = texture2D(tex_sampler,gl_TexCoord[0].st);"
			"}";

		glShaderSource(v_shader, 1, &v_shader_src, 0);
		glShaderSource(f_shader, 1, &f_shader_src, 0);
		glCompileShader(v_shader);
		glCompileShader(f_shader);

		this->program = glCreateProgram();
		glAttachShader(this->program, v_shader);
		glAttachShader(this->program, f_shader);
		glLinkProgram(this->program);

		glDeleteShader(v_shader);
		glDeleteShader(f_shader);
	}

	void canvas::prepare(HDC _hdc)
	{
		bool reset = this->hdc != _hdc || !this->is_available() || glIsTexture(this->texture) == GL_FALSE;

		if (!reset)
		{
			POINT dim;
			canvas::get_dimension(_hdc, &dim);

			reset = uint32_t(dim.x) != this->get_width() || uint32_t(dim.y) != this->get_height();
		}

		if (reset)
		{
			this->release();
			this->create(_hdc);
		}
	}

	void canvas::draw_internal()
	{
		this->apply_update();

		POINT dim;
		canvas::get_dimension(this->hdc, &dim);

		int s_width = dim.x;
		int s_weight = dim.y;

		int width = this->get_width();
		int height = this->get_height();

		int x = 0;
		int y = 0;

		DWORD color = 0xFFFFFFFF;

		glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
		glPushAttrib(GL_ALL_ATTRIB_BITS);

		GLint id;
		glGetIntegerv(GL_CURRENT_PROGRAM, &id);

		glUseProgram(this->program);

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0.0, (double)s_width, (double)s_weight, 0.0, -1.0, 1.0);
		glTranslatef(0.0, 0.0, 0.0);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		glEnable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glColor4d(GetRValue(color) / 255.0, GetGValue(color) / 255.0, GetBValue(color) / 255.0, (LOBYTE((color) >> 24)) / 255.0);

		GLint texture2d;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture2d);
		glBindTexture(GL_TEXTURE_2D, this->texture);

		glBegin(GL_QUADS);
		glTexCoord2i(0, 0); glVertex3i(x, y, 0);
		glTexCoord2i(0, 1); glVertex3i(x, (height + y), 0);
		glTexCoord2i(1, 1); glVertex3i((width + x), (height + y), 0);
		glTexCoord2i(1, 0); glVertex3i((width + x), y, 0);
		glEnd();

		glBindTexture(GL_TEXTURE_2D, texture2d);

		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);

		glUseProgram(id);

		glPopAttrib();
		glPopClientAttrib();
	}

	void canvas::draw(HDC _hdc)
	{
		if (!_hdc) return;
		this->prepare(_hdc);

		this->hdc = _hdc;
		this->draw_internal();
	}

	void canvas::release()
	{
		this->hdc = nullptr;

		if (this->texture)
		{
			glDeleteTextures(1, &this->texture);
			this->texture = NULL;
		}

		if (this->program)
		{
			glDeleteProgram(this->program);
			this->program = NULL;
		}

		if (this->buffer)
		{
			delete[] this->buffer;
			this->buffer = nullptr;
		}
	}

	void canvas::get_dimension(HDC hdc, LPPOINT point)
	{
		if (!point || hdc == nullptr) return;

		HWND window = WindowFromDC(hdc);

		RECT rect;
		GetClientRect(window, &rect);

		point->x = std::abs(rect.left - rect.right);
		point->y = std::abs(rect.bottom - rect.top);
	}
}
