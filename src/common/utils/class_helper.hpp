#pragma once

#include <optional>
#include <functional>

#define CLASS_DISABLE_COPY(cls) \
    cls(const cls&) = delete;   \
    cls& operator=(const cls&) = delete

#define CLASS_DISABLE_MOVE(cls) \
    cls(cls&&) = delete;        \
    cls& operator=(cls&&) = delete

#define CLASS_DISABLE_COPY_AND_MOVE(cls) \
    CLASS_DISABLE_MOVE(cls);             \
    CLASS_DISABLE_COPY(cls)

#define CLASS_DECLARE_INTERFACE(cls) \
    cls() = default;                 \
    virtual ~cls() = default;        \
    CLASS_DISABLE_COPY_AND_MOVE(cls)

namespace utils
{
    struct object
    {
        object() = default;
        virtual ~object() = default;
    };

    template <typename T>
        requires(std::is_trivially_copyable_v<T>) // TODO: Get rid of that once needed
    class owned_object
    {
      public:
        owned_object() = default;
        owned_object(T object, std::function<void(T)> destructor)
            : destructor_(std::move(destructor)),
              object_(std::move(object))
        {
        }

        ~owned_object()
        {
            this->destroy();
        }

        owned_object(const owned_object&) = delete;
        owned_object& operator=(const owned_object&) = delete;

        owned_object(owned_object&& obj) noexcept
        {
            this->operator=(std::move(obj));
        }

        owned_object& operator=(owned_object&& obj) noexcept
        {
            if (this != &obj)
            {
                this->destroy();

                this->object_ = std::move(obj.object_);
                this->destructor_ = std::move(obj.destructor_);

                obj.destructor_ = {};
                obj.object_ = {};
            }

            return *this;
        }

        T get() const
        {
            return *this->object_;
        }

        operator T() const
        {
            return this->get();
        }

        bool is_valid() const
        {
            return this->object_.has_value();
        }

      private:
        std::function<void(T)> destructor_{};
        std::optional<T> object_{};

        void destroy()
        {
            if (!this->object_)
            {
                return;
            }

            if (!this->destructor_)
            {
                std::abort();
            }

            this->destructor_(*this->object_);

            this->destructor_ = {};
            this->object_ = {};
        }
    };

}
