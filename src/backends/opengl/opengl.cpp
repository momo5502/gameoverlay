#include "std_include.hpp"
#include "../../utils/hook.hpp"
#include "../../gameoverlay.hpp"
#include "../../utils/concurrency.hpp"
#include "opengl_renderer.hpp"

#include <shared_mutex>

namespace gameoverlay::opengl
{
	namespace
	{
		using shared_lock_t = std::shared_lock<std::shared_mutex>;
		using renderer_map = std::unordered_map<HDC, std::unique_ptr<renderer>>;
		utils::concurrency::container<renderer_map, std::shared_mutex> renderers{};

		utils::hook::detour swap_buffers_hook;
		utils::hook::detour delete_dc_hook;
		utils::hook::detour destroy_window_hook;

		bool try_access_renderer(const HDC hdc, const std::function<void(renderer&)>& callback = {})
		{
			return renderers.access<bool, shared_lock_t>([hdc, &callback](const renderer_map& r)
			{
				const auto entry = r.find(hdc);
				if (entry != r.end())
				{
					if (callback)
					{
						callback(*entry->second);
					}
					return true;
				}

				return false;
			});
		}

		bool has_renderer(const HWND window)
		{
			return renderers.access<bool, shared_lock_t>([window](const renderer_map& r)
			{
				for (auto i = r.begin(); i != r.end(); ++i)
				{
					const auto entry_window = WindowFromDC(i->first);
					if (entry_window == window)
					{
						return true;
					}
				}

				return false;
			});
		}

		void access_renderer(const HDC hdc, const std::function<void(renderer&)>& callback)
		{
			if (try_access_renderer(hdc, callback))
			{
				return;
			}

			renderers.access([hdc](renderer_map& r)
			{
				r[hdc] = std::make_unique<renderer>(hdc);
			});

			if (!try_access_renderer(hdc, callback))
			{
				throw std::runtime_error("WAT?");
			}
		}

		void draw_frame(const HDC hdc)
		{
			access_renderer(hdc, [](renderer& renderer)
			{
				renderer.draw_frame();
			});
		}

		BOOL WINAPI swap_buffers_stub(const HDC hdc)
		{
			draw_frame(hdc);
			return swap_buffers_hook.invoke_stdcall<BOOL>(hdc);
		}

		BOOL WINAPI delete_dc_stub(const HDC hdc)
		{
			if (try_access_renderer(hdc))
			{
				renderers.access([hdc](renderer_map& r)
				{
					const auto entry = r.find(hdc);
					if (entry != r.end())
					{
						r.erase(entry);
					}
				});
			}

			return delete_dc_hook.invoke_stdcall<BOOL>(hdc);
		}

		BOOL WINAPI destroy_window_stub(HWND window)
		{
			if (has_renderer(window))
			{
				renderers.access([window](renderer_map& r)
				{
					for (auto i = r.begin(); i != r.end();)
					{
						const auto entry_window = WindowFromDC(i->first);
						if (entry_window == window)
						{
							i = r.erase(i);
						}
						else
						{
							++i;
						}
					}
				});
			}

			return destroy_window_hook.invoke_stdcall<BOOL>(window);
		}
	}

	void initialize()
	{
		swap_buffers_hook.create(::SwapBuffers, swap_buffers_stub);
		delete_dc_hook.create(::DeleteDC, delete_dc_stub);
		destroy_window_hook.create(::DestroyWindow, destroy_window_stub);
	}

	static register_backend _(initialize);
}
