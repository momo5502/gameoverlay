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
		using renderer_map = std::unordered_map<HDC, std::unique_ptr<renderer>>;
		utils::concurrency::container<renderer_map> renderers;
		std::shared_mutex mutex;

		utils::hook::detour swap_buffers_hook;
		utils::hook::detour delete_dc_hook;

		renderer& get_renderer(const HDC hdc)
		{
			return *renderers.access<renderer*>([hdc](renderer_map& r)
			{
				const auto entry = r.find(hdc);
				if (entry != r.end())
				{
					return entry->second.get();
				}

				auto new_renderer = std::make_unique<renderer>(hdc);
				auto* renderer_ptr = new_renderer.get();
				r[hdc] = std::move(new_renderer);

				return renderer_ptr;
			});
		}

		void draw_frame(const HDC hdc)
		{
			std::shared_lock _(mutex);

			auto& renderer = get_renderer(hdc);
			renderer.draw_frame();
		}

		BOOL WINAPI swap_buffers_stub(const HDC hdc)
		{
			draw_frame(hdc);
			return static_cast<decltype(::SwapBuffers)*>(swap_buffers_hook.get_original())(hdc);
		}

		BOOL WINAPI delete_dc_stub(const HDC hdc)
		{
			renderers.access([hdc](renderer_map& r)
			{
				const auto entry = r.find(hdc);
				if (entry != r.end())
				{
					std::lock_guard _(mutex);
					r.erase(entry);
				}
			});

			return static_cast<decltype(::DeleteDC)*>(delete_dc_hook.get_original())(hdc);
		}
	}

	void initialize()
	{
		swap_buffers_hook.create(::SwapBuffers, swap_buffers_stub);
		delete_dc_hook.create(::DeleteDC, delete_dc_stub);
	}

	static register_backend _(initialize);
}
