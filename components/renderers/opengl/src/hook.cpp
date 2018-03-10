#include "std_include.hpp"

#include "hook.hpp"

namespace gameoverlay
{
	hook* hook::instance = nullptr;

	void hook::frame_handler(HDC hdc)
	{
		if (!hdc) return;
		if (this->frame_callback) this->frame_callback(hdc);
	}

	void hook::on_frame(std::function<void(HDC)> callback)
	{
		this->frame_callback = callback;
	}

	void hook::unhook()
	{

	}

	BOOL WINAPI hook::swapBuffers_stub(HDC hdc)
	{
		MessageBoxA(0, 0, 0, 0);
		bool result = hook::instance->swapBuffers_hook.invoke_pascal<bool>(hdc);
		return result;
	}

	hook::hook()
	{
		hook::instance = this;

		this->swapBuffers_hook.create(::SwapBuffers, hook::swapBuffers_stub);
	}

	hook::~hook()
	{
		hook::instance = nullptr;
	}
}
