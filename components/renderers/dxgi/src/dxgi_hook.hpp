#pragma once

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

		typedef HRESULT(WINAPI* present_t)(void*, UINT, UINT);
		present_t original_present;
		void* present_hook;

		HRESULT WINAPI present(void* swap_chain, UINT sync_interval, UINT flags);
		static HRESULT WINAPI present_stub(void* swap_chain, UINT sync_interval, UINT flags);
	};
}
