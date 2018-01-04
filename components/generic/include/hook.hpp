#pragma once

#include <MinHook.h>

namespace gameoverlay
{
	namespace utils
	{
		class hook
		{
		public:
			hook();
			~hook();

			void create(void* target, void* function);
			void remove();

			void enable();
			void disable();

			template<typename T, typename... Args>
			T invoke(Args... args)
			{
				return reinterpret_cast<T(__cdecl*)(Args...)>(this->original)(args...);
			}

			template<typename T, typename... Args>
			T invoke_pascal(Args... args)
			{
				return reinterpret_cast<T(__stdcall*)(Args...)>(this->original)(args...);
			}

			template<typename T, typename... Args>
			T invoke_this(void* thisPtr, Args... args)
			{
				return reinterpret_cast<T(__thiscall*)(Args...)>(this->original)(thisPtr, args...);
			}

		private:
			void* target;
			void* original;

			class initializer
			{
			public:
				initializer();
				~initializer();
			};
		};
	}
}