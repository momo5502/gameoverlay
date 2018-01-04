#pragma once

#include <hook.hpp>

namespace gameoverlay
{
	class renderer;
	class dxgi_hook
	{
	public:
		dxgi_hook();
		~dxgi_hook();

		void on_frame(std::function<void(void*)> callback);

	private:
		static dxgi_hook* instance;
		std::function<void(void*)> callback;

		utils::hook present_hook;

		HRESULT WINAPI present(void* swap_chain, UINT sync_interval, UINT flags);
		static HRESULT WINAPI present_stub(void* swap_chain, UINT sync_interval, UINT flags);
	};
}
