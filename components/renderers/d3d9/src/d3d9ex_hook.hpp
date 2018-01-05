#pragma once

#include <hook.hpp>

namespace gameoverlay
{
	class d3d9ex_hook
	{
	public:
		d3d9ex_hook();
		~d3d9ex_hook();

		void on_frame(std::function<void(void*)> callback);
		void unhook();

	private:
		static d3d9ex_hook* instance;

		std::function<void(void*)> callback;

		utils::hook endscene_hook;
		utils::hook reset_hook;
		utils::hook present_hook;
		utils::hook presentex_hook;
		utils::hook swap_chain_present_hook;

		void frame_handler(void* device);

		HRESULT WINAPI reset(void* device, void* presentation_parameters);
		static HRESULT WINAPI reset_stub(void* device, void* presentation_parameters);

		HRESULT WINAPI swap_chain_present(void* swap_chain, CONST RECT* source_rect, CONST RECT* dest_rect, HWND dest_window_override, CONST RGNDATA* dirty_region, DWORD flags);
		static HRESULT WINAPI swap_chain_present_stub(void* swap_chain, CONST RECT* source_rect, CONST RECT* dest_rect, HWND dest_window_override, CONST RGNDATA* dirty_region, DWORD flags);

		HRESULT WINAPI present(void* device, CONST RECT* source_rect, CONST RECT* dest_rect, HWND dest_window_override, CONST RGNDATA* dirty_region);
		static HRESULT WINAPI present_stub(void* device, CONST RECT* source_rect, CONST RECT* dest_rect, HWND dest_window_override, CONST RGNDATA* dirty_region);

		HRESULT WINAPI presentex(void* device, CONST RECT* source_rect, CONST RECT* dest_rect, HWND dest_window_override, CONST RGNDATA* dirty_region, DWORD flags);
		static HRESULT WINAPI presentex_stub(void* device, CONST RECT* source_rect, CONST RECT* dest_rect, HWND dest_window_override, CONST RGNDATA* dirty_region, DWORD flags);

		HRESULT WINAPI endscene(void* device);
		static HRESULT WINAPI endscene_stub(void* device);
	};
}
