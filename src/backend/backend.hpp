#pragma once

#include <unordered_map>
#include <utils/win.hpp>
#include <utils/concurrency.hpp>
#include <utils/class_helper.hpp>

#include "backend_type.hpp"
#include "renderer.hpp"

namespace gameoverlay
{
    class backend
    {
      public:
        struct handler
        {
            CLASS_DECLARE_INTERFACE(handler);

            virtual std::unique_ptr<renderer::handler> create_renderer_handler() = 0;

            virtual void on_new_renderer(renderer& r)
            {
                (void)r;
            }
        };

        using owned_handler = std::unique_ptr<handler>;

        backend(owned_handler h)
            : handler_(std::move(h))
        {
        }

        CLASS_DISABLE_COPY_AND_MOVE(backend);

        virtual ~backend() = default;

        virtual void on_window_destruction(const HWND window)
        {
            (void)window;
        }

      protected:
        owned_handler handler_{};
    };

    template <typename Renderer, typename RendererKey>
        requires(std::is_base_of_v<renderer, Renderer>)
    class typed_backed : public backend
    {
      public:
        using backend::backend;

        void on_window_destruction(const HWND window) override
        {
            this->renderers_.access([&](renderers& m) {
                std::erase_if(m, [&](const typename renderers::value_type& entry) {
                    return entry.second->get_window() == window; //
                });
            });
        }

        template <typename Accessor>
            requires(std::is_invocable_v<Accessor, Renderer&>)
        bool access_renderer(const RendererKey& key, const Accessor& accessor)
        {
            return this->renderers_.template access<bool>([&](renderers& r) {
                const auto entry = r.find(key);
                if (entry == r.end())
                {
                    return false;
                }

                accessor(static_cast<Renderer&>(*entry->second));
                return true;
            });
        }

        template <typename Accessor, typename... Args>
            requires(std::is_invocable_v<Accessor, Renderer&>)
        bool create_or_access_renderer(RendererKey key, const Accessor& accessor, Args&&... args)
        {
            return this->renderers_.template access<bool>([&](renderers& r) {
                const auto entry = r.find(key);
                if (entry != r.end())
                {
                    accessor(static_cast<Renderer&>(*entry->second));
                    return false;
                }

                auto renderer = this->construct_renderer(std::forward<Args>(args)...);
                auto* ptr = this->store_renderer(r, std::move(key), std::move(renderer));

                accessor(*ptr);

                return true;
            });
        }

        // Are these really needed?

        template <typename Accessor, typename... Args>
            requires(std::is_invocable_v<Accessor, Renderer&>)
        void create_and_access_renderer(RendererKey key, const Accessor& accessor, Args&&... args)
        {
            auto renderer = this->construct_renderer(std::forward<Args>(args)...);
            this->renderers_.access([&](renderers& r) {
                auto* ptr = this->store_renderer(r, std::move(key), std::move(renderer));
                accessor(*ptr);
            });
        }

        template <typename... Args>
        void create_renderer(RendererKey key, Args&&... args)
        {
            this->create_and_access_renderer(std::move(key), [](Renderer&) {}, std::forward<Args>(args)...);
        }

        template <typename Predicate = bool (*)()>
            requires(std::is_invocable_r_v<bool, Predicate>)
        bool erase(const RendererKey& key, const Predicate& guardedPredicate = +[] { return true; })
        {
            return this->renderers_.template access<bool>([&](renderers& r) {
                const auto canErase = guardedPredicate();
                if (!canErase)
                {
                    return false;
                }

                const auto entry = r.find(key);

                if (entry == r.end())
                {
                    return false;
                }

                r.erase(entry);
                return true;
            });
        }

      private:
        using renderers = std::unordered_map<RendererKey, std::unique_ptr<renderer>>;
        utils::concurrency::container<renderers, std::recursive_mutex> renderers_{};

        Renderer* store_renderer(renderers& r, RendererKey key, std::unique_ptr<Renderer> renderer)
        {
            if (!renderer)
            {
                throw std::runtime_error("Bad renderer provided");
            }

            auto* renderer_ptr = renderer.get();

            r.emplace(std::move(key), std::move(renderer));
            this->handler_->on_new_renderer(*renderer_ptr);

            return renderer_ptr;
        }

        template <typename... Args>
        std::unique_ptr<Renderer> construct_renderer(Args&&... args)
        {
            auto h = this->handler_->create_renderer_handler();
            return std::make_unique<Renderer>(std::move(h), std::forward<Args>(args)...);
        }
    };
}
