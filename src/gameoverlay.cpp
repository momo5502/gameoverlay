#include <std_include.hpp>
#include "gameoverlay.hpp"
#include "utils/concurrency.hpp"
#include <thread>

namespace gameoverlay
{
	using backend_initializers = std::vector<std::function<void()>>;

	/*****************************************************************************
	 *
	 ****************************************************************************/

	utils::concurrency::container<backend_initializers>& get_backends()
	{
		static utils::concurrency::container<backend_initializers> backends{};
		return backends;
	}

	/*****************************************************************************
	 *
	 ****************************************************************************/

	register_backend::register_backend(std::function<void()> backend)
	{
		get_backends().access([&](backend_initializers& backends)
		{
			backends.emplace_back(std::move(backend));
		});
	}

	/*****************************************************************************
	 *
	 ****************************************************************************/

	void initialize()
	{
		get_backends().access([&](backend_initializers& backends)
		{
			for (auto& initializer : backends)
			{
				initializer();
			}

			backends.clear();
		});
	}

	namespace
	{
		/*****************************************************************************
		 *
		 ****************************************************************************/

		struct initializer
		{
			initializer()
			{
				std::thread([]
				{
					std::this_thread::sleep_for(std::chrono::seconds(1));
					initialize();
				}).detach();
			}
		} _;
	}
}
