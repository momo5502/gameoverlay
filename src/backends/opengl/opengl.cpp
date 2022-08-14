#include "std_include.hpp"

#include "../../utils/hook.hpp"
#include "../../utils/finally.hpp"
#include "../../utils/concurrent_map.hpp"

#include "../../backend_registry.hpp"

#include "opengl_renderer.hpp"


namespace gameoverlay::opengl
{
	namespace
	{
		utils::concurrency::map<HDC, std::unique_ptr<renderer>> renderers{};

		utils::hook::detour swap_buffers_hook;
		utils::hook::detour delete_dc_hook;

		void draw_frame(const HDC hdc)
		{
			renderers.access(hdc, [](const std::unique_ptr<renderer>& renderer)
			{
				renderer->draw_frame();
			}, [hdc]
			{
				return std::make_unique<renderer>(hdc);
			});
		}

		BOOL WINAPI swap_buffers_stub(const HDC hdc)
		{
			draw_frame(hdc);
			return swap_buffers_hook.invoke_stdcall<BOOL>(hdc);
		}

		BOOL WINAPI delete_dc_stub(const HDC hdc)
		{
			renderers.remove(hdc);
			return delete_dc_hook.invoke_stdcall<BOOL>(hdc);
		}

		class backend : public ::gameoverlay::backend
		{
			void initialize() override
			{
				swap_buffers_hook.create(::SwapBuffers, swap_buffers_stub);
				delete_dc_hook.create(::DeleteDC, delete_dc_stub);
			}

			void on_window_destruction(const HWND window) override
			{
				const auto dc = GetDC(window);
				const auto _ = utils::finally([&] { ReleaseDC(window, dc); });

				renderers.remove(dc);
			}
		};
	}
}

REGISTER_BACKEND(gameoverlay::opengl::backend);
