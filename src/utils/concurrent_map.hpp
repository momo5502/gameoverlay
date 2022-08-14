#pragma once

#include <unordered_map>
#include <shared_mutex>
#include <functional>

#include "concurrency.hpp"

namespace utils::concurrency
{
	template <typename Key, typename Value>
	class map
	{
	public:
		void access(const Key& key, const std::function<void(Value&)>& callback,
		            const std::function<Value()>& constructor)
		{
			if (this->try_access(key, callback))
			{
				return;
			}

			this->container_.access([&](map_type& m)
			{
				m[key] = constructor();
			});

			if (!this->try_access(key, callback))
			{
				throw std::runtime_error("Element not contained after construction. This should never happen");
			}
		}

		void remove(const Key& key)
		{
			if (!this->contains(key)) return;

			this->container_.access([&](map_type& m)
			{
				auto entry = m.find(key);
				if (entry != m.end())
				{
					m.erase(entry);
				}
			});
		}

		void remove_if(const std::function<bool(const Key&, Value&)> predicate)
		{
			this->container_.access([&](map_type& m)
			{
				for (auto i = m.begin(); i != m.end();)
				{
					if (predicate(i->first, i->second))
					{
						i = m.erase(i);
					}
					else
					{
						++i;
					}
				}
			});
		}

	private:
		using shared_lock_t = std::shared_lock<std::shared_mutex>;
		using map_type = std::unordered_map<Key, Value>;
		utils::concurrency::container<map_type, std::shared_mutex> container_{};

		bool contains(const Key& key)
		{
			return try_access(key, {});
		}

		bool try_access(const Key& key, const std::function<void(Value&)>& callback)
		{
			return this->container_.template access<bool, shared_lock_t>([&](map_type& m)
			{
				const auto entry = m.find(key);
				if (entry != m.end())
				{
					if (callback)
					{
						callback(entry->second);
					}

					return true;
				}

				return false;
			});
		}
	};
}
