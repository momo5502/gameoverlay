#include "std_include.hpp"
#include "opengl_canvas.hpp"

#include "../../utils/finally.hpp"

#include <memory>
#include <vector>

#define GLSL(version, shader)  "#version " #version "\n" #shader

namespace gameoverlay::opengl
{
	namespace
	{
		void show_compilation_error(const GLint shader)
		{
			GLint is_compiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &is_compiled);
			if (is_compiled == GL_FALSE)
			{
				GLint max_length = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_length);

				// The maxLength includes the NULL character
				std::vector<GLchar> error_log(max_length);
				glGetShaderInfoLog(shader, max_length, &max_length, error_log.data());

				MessageBoxA(0, error_log.data(), "Error compiling shader", MB_ICONERROR);
			}
		}

		GLuint create_shader()
		{
			const GLint v_shader = glCreateShader(GL_VERTEX_SHADER);
			auto v_shader_del = utils::finally([&v_shader] { glDeleteShader(v_shader); });

			const GLint f_shader = glCreateShader(GL_FRAGMENT_SHADER);
			auto f_shader_del = utils::finally([&f_shader] { glDeleteShader(f_shader); });

			auto* v_shader_src =
				"void main(void)"
				"{"
				"	gl_TexCoord[0] = gl_MultiTexCoord0;"
				"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
				"}";

			auto* f_shader_src =
				"uniform sampler2D tex_sampler;"
				"void main(void)"
				"{"
				"	gl_FragColor = texture2D(tex_sampler,gl_TexCoord[0].st);"
				"}";

			glShaderSource(v_shader, 1, &v_shader_src, nullptr);
			glShaderSource(f_shader, 1, &f_shader_src, nullptr);
			glCompileShader(v_shader);
			show_compilation_error(v_shader);
			glCompileShader(f_shader);
			show_compilation_error(f_shader);

			const auto program = glCreateProgram();
			glAttachShader(program, v_shader);
			glAttachShader(program, f_shader);
			glLinkProgram(program);

			return program;
		}
	}

	GLuint create_texture(const uint32_t width, const uint32_t height)
	{
		GLint texture2d{};
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture2d);
		auto _1 = utils::finally([&texture2d] { glBindTexture(GL_TEXTURE_2D, texture2d); });

		int alignment = 0;
		glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		auto _2 = utils::finally([&alignment] { glPixelStorei(GL_UNPACK_ALIGNMENT, alignment); });

		GLuint new_texture{};
		glGenTextures(1, &new_texture);
		glBindTexture(GL_TEXTURE_2D, new_texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		const auto bytes = width * height * 4;
		const std::unique_ptr<uint8_t[]> buffer(new uint8_t[bytes]);
		memset(buffer.get(), 0xFF, bytes);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, GL_RGBA,
		             GL_UNSIGNED_BYTE, buffer.get());

		return new_texture;
	}

	canvas::canvas(const uint32_t width, const uint32_t height)
		: width_(width),
		  height_(height),
		  texture_(create_texture(width, height)),
		  program_(create_shader())
	{
	}

	canvas::~canvas()
	{
		if (this->texture_)
		{
			glDeleteTextures(1, &this->texture_);
		}

		if (this->program_)
		{
			glDeleteProgram(this->program_);
		}
	}

	dimensions canvas::get_dimensions() const
	{
		return {this->width_, this->height_};
	}

	void canvas::paint(const void* image)
	{
		GLint texture2d;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture2d);
		auto _1 = utils::finally([&texture2d] { glBindTexture(GL_TEXTURE_2D, texture2d); });

		glBindTexture(GL_TEXTURE_2D, this->texture_);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, static_cast<GLsizei>(this->width_), static_cast<GLsizei>(this->height_),
		                GL_RGBA, GL_UNSIGNED_BYTE, image);
	}

	void canvas::draw() const
	{
		glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
		auto _1 = utils::finally([] { glPopClientAttrib(); });

		glPushAttrib(GL_ALL_ATTRIB_BITS);
		auto _2 = utils::finally([] { glPopAttrib(); });

		GLint id;
		glGetIntegerv(GL_CURRENT_PROGRAM, &id);
		auto _3 = utils::finally([&id] { glUseProgram(id); });

		glUseProgram(this->program_);

		glMatrixMode(GL_TEXTURE);
		glPushMatrix();
		glLoadIdentity();
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(-1, 1, -1, 1, -1, 2);
		glTranslatef(0.0, 0.0, 0.0);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		auto _4 = utils::finally([]
		{
			glPopMatrix();
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_TEXTURE);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
		});

		glEnable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glCullFace(GL_FRONT_AND_BACK);

		glColor4d(1.0, 1.0, 1.0, 1.0);

		GLint texture2d;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &texture2d);
		auto _5 = utils::finally([&texture2d] { glBindTexture(GL_TEXTURE_2D, texture2d); });

		glBindTexture(GL_TEXTURE_2D, this->texture_);
		glActiveTexture(GL_TEXTURE0);

		glBegin(GL_QUADS);
		glTexCoord2i(0, 0);
		glVertex3i(-1, -1, 1);
		glTexCoord2i(0, 1);
		glVertex3i(-1, 1, 1);
		glTexCoord2i(1, 1);
		glVertex3i(1, 1, 1);
		glTexCoord2i(1, 0);
		glVertex3i(1, -1, 1);
		glEnd();
	}
}
