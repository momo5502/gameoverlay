#pragma once

#include <hook.hpp>

namespace gameoverlay
{
	class renderer;
	class hook_dxgi
	{
	public:
		hook_dxgi();
		~hook_dxgi();

		void on_frame(std::function<void(void*)> callback);

	private:
		static hook_dxgi* instance;
		std::function<void(void*)> callback;

		utils::hook present_hook;

		HRESULT WINAPI present(void* swap_chain, UINT sync_interval, UINT flags);
		static HRESULT WINAPI present_stub(void* swap_chain, UINT sync_interval, UINT flags);
	};
}
