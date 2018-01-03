#pragma once

namespace gameoverlay
{
	class renderer;
	class dxgi_hook
	{
	public:
		dxgi_hook(renderer* parent);
		~dxgi_hook();

	private:
		static dxgi_hook* instance;

		renderer* parent;

		typedef HRESULT(WINAPI* present_t)(void*, UINT, UINT);
		present_t original_present;
		void* present_hook;

		HRESULT WINAPI present(void* swap_chain, UINT sync_interval, UINT flags);
		static HRESULT WINAPI present_stub(void* swap_chain, UINT sync_interval, UINT flags);
	};
}
