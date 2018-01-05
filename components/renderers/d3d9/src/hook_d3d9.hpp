#pragma once

#include <hook.hpp>

namespace gameoverlay
{
	class hook_d3d9
	{
	public:
		hook_d3d9();
		~hook_d3d9();

		void on_frame(std::function<void(void*)> callback);
		void on_reset(std::function<void(void*)> callback);
		void unhook();

	private:
		static hook_d3d9* instance;

		std::function<void(void*)> frame_callback;
		std::function<void(void*)> reset_callback;

		utils::hook endscene_hook;
		utils::hook reset_hook;
		utils::hook present_hook;
		utils::hook swap_chain_present_hook;

		void frame_handler(void* device);
		void reset_handler(void* device);

		HRESULT WINAPI reset(void* device, void* presentation_parameters);
		static HRESULT WINAPI reset_stub(void* device, void* presentation_parameters);

		HRESULT WINAPI swap_chain_present(void* swap_chain, CONST RECT* source_rect, CONST RECT* dest_rect, HWND dest_window_override, CONST RGNDATA* dirty_region, DWORD flags);
		static HRESULT WINAPI swap_chain_present_stub(void* swap_chain, CONST RECT* source_rect, CONST RECT* dest_rect, HWND dest_window_override, CONST RGNDATA* dirty_region, DWORD flags);

		HRESULT WINAPI present(void* device, CONST RECT* source_rect, CONST RECT* dest_rect, HWND dest_window_override, CONST RGNDATA* dirty_region);
		static HRESULT WINAPI present_stub(void* device, CONST RECT* source_rect, CONST RECT* dest_rect, HWND dest_window_override, CONST RGNDATA* dirty_region);

		HRESULT WINAPI endscene(void* device);
		static HRESULT WINAPI endscene_stub(void* device);
	};
}
