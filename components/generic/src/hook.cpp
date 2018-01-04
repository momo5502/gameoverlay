#pragma once

#include "hook.hpp"

namespace gameoverlay
{
	namespace utils
	{
		hook::initializer::initializer()
		{
			MH_Initialize();
		}

		hook::initializer::~initializer()
		{
			MH_Uninitialize();
		}

		hook::hook() : target(nullptr), original(nullptr)
		{
			static hook::initializer _;
		}

		hook::~hook()
		{
			this->remove();
		}

		void hook::create(void* _target, void* function)
		{
			this->remove();

			this->target = _target;
			MH_CreateHook(this->target, function, &this->original);

			this->enable();
		}

		void hook::remove()
		{
			if (this->target)
			{
				MH_RemoveHook(this->target);
				this->target = nullptr;
			}
		}

		void hook::enable()
		{
			if (this->target)
			{
				MH_EnableHook(this->target);
			}
		}

		void hook::disable()
		{
			if (this->target)
			{
				MH_DisableHook(this->target);
			}
		}
	}
}
