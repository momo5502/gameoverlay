#include <std_include.hpp>
#include "backend_registry.hpp"

#include <vector>

#include "utils/concurrency.hpp"

namespace gameoverlay::backend_registry
{
	using backend_vector = std::vector<std::unique_ptr<backend>>;

	/*****************************************************************************
	 *
	 ****************************************************************************/

	utils::concurrency::container<backend_vector>& get_backends()
	{
		static utils::concurrency::container<backend_vector> backends{};
		return backends;
	}

	/*****************************************************************************
	 *
	 ****************************************************************************/

	void register_backend(std::unique_ptr<backend>&& b)
	{
		get_backends().access([&b](backend_vector& backends)
		{
			backends.emplace_back(std::move(b));
		});
	}

	/*****************************************************************************
	 *
	 ****************************************************************************/

	void initialize()
	{
		get_backends().access([](const backend_vector& backends)
		{
			for (const auto& backend : backends)
			{
				backend->initialize();
			}
		});
	}

	/*****************************************************************************
	 *
	 ****************************************************************************/

	void on_window_destruction(const HWND window)
	{
		get_backends().access([&](const backend_vector& backends)
		{
			for (const auto& backend : backends)
			{
				backend->on_window_destruction(window);
			}
		});
	}
}
