#pragma once

#include "win.hpp"

#include <string>
#include <functional>
#include <filesystem>

namespace utils::nt
{
    class library final
    {
      public:
        static library load(const char* name);
        static library load(const std::string& name);
        static library load(const std::filesystem::path& path);
        static library get_by_address(const void* address);

        library();
        explicit library(const std::string& name);
        explicit library(HMODULE handle);

        library(const library& a)
            : module_(a.module_)
        {
        }

        bool operator!=(const library& obj) const
        {
            return !(*this == obj);
        };
        bool operator==(const library& obj) const;

        operator bool() const;
        operator HMODULE() const;

        void unprotect() const;
        [[nodiscard]] void* get_entry_point() const;
        [[nodiscard]] size_t get_relative_entry_point() const;

        [[nodiscard]] bool is_valid() const;
        [[nodiscard]] std::string get_name() const;
        [[nodiscard]] std::filesystem::path get_path() const;
        [[nodiscard]] std::filesystem::path get_folder() const;
        [[nodiscard]] std::uint8_t* get_ptr() const;
        void free();

        [[nodiscard]] HMODULE get_handle() const;

        template <typename T>
        [[nodiscard]] T get_proc(const char* process) const
        {
            if (!this->is_valid())
                T{};
            return reinterpret_cast<T>(GetProcAddress(this->module_, process));
        }

        template <typename T>
        [[nodiscard]] T get_proc(const std::string& process) const
        {
            return get_proc<T>(process.data());
        }

        template <typename T>
        [[nodiscard]] std::function<T> get(const std::string& process) const
        {
            if (!this->is_valid())
                return std::function<T>();
            return static_cast<T*>(this->get_proc<void*>(process));
        }

        template <typename T, typename... Args>
        T invoke(const std::string& process, Args... args) const
        {
            auto method = this->get<T(__cdecl)(Args...)>(process);
            if (method)
                return method(args...);
            return T();
        }

        template <typename T, typename... Args>
        T invoke_pascal(const std::string& process, Args... args) const
        {
            auto method = this->get<T(__stdcall)(Args...)>(process);
            if (method)
                return method(args...);
            return T();
        }

        template <typename T, typename... Args>
        T invoke_this(const std::string& process, void* this_ptr, Args... args) const
        {
            auto method = this->get<T(__thiscall)(void*, Args...)>(this_ptr, process);
            if (method)
                return method(args...);
            return T();
        }

        [[nodiscard]] std::vector<PIMAGE_SECTION_HEADER> get_section_headers() const;

        [[nodiscard]] PIMAGE_NT_HEADERS get_nt_headers() const;
        [[nodiscard]] PIMAGE_DOS_HEADER get_dos_header() const;
        [[nodiscard]] PIMAGE_OPTIONAL_HEADER get_optional_header() const;

        [[nodiscard]] void** get_iat_entry(const std::string& module_name, std::string proc_name) const;
        [[nodiscard]] void** get_iat_entry(const std::string& module_name, const char* proc_name) const;

      private:
        HMODULE module_;
    };

    inline HANDLE null_handle_value()
    {
        return nullptr;
    }

    inline HANDLE invalid_handle_value()
    {
        return INVALID_HANDLE_VALUE;
    }

    template <HANDLE(InvalidHandleProvider)()>
    class handle
    {
      public:
        handle() = default;

        handle(const HANDLE h)
            : handle_(h)
        {
        }

        ~handle()
        {
            this->close();
        }

        handle(const handle& obj)
            : handle()
        {
            this->operator=(obj);
        }

        handle& operator=(const handle& obj)
        {
            if (this != &obj)
            {
                this->close();

                if (!obj)
                {
                    return *this;
                }

                const auto res = DuplicateHandle(GetCurrentProcess(), obj.handle_, GetCurrentProcess(), &this->handle_,
                                                 0, FALSE, DUPLICATE_SAME_ACCESS);

                if (!res)
                {
                    this->handle_ = InvalidHandleProvider();
                    throw std::runtime_error("Failed to duplicate handle");
                }
            }

            return *this;
        }

        handle(handle&& obj) noexcept
            : handle()
        {
            this->operator=(std::move(obj));
        }

        handle& operator=(handle&& obj) noexcept
        {
            if (this != &obj)
            {
                this->close();
                this->handle_ = obj.handle_;
                obj.handle_ = InvalidHandleProvider();
            }

            return *this;
        }

        handle& operator=(HANDLE h) noexcept
        {
            this->close();
            this->handle_ = h;

            return *this;
        }

        bool operator==(const HANDLE h) const
        {
            if (this->handle_ == h)
            {
                return true;
            }

            if (!*this || h == InvalidHandleProvider())
            {
                return false;
            }

            return CompareObjectHandles(this->handle_, h) != FALSE;
        }

        bool operator!=(const HANDLE h) const
        {
            return !(*this == h);
        }

        bool operator==(const handle& h) const
        {
            return this->operator==(h.handle_);
        }

        bool operator!=(const handle& h) const
        {
            return !(*this == h);
        }

        [[nodiscard]] explicit operator bool() const
        {
            return this->handle_ != InvalidHandleProvider();
        }

        [[nodiscard]] operator HANDLE() const
        {
            return this->handle_;
        }

        void close()
        {
            if (*this)
            {
                CloseHandle(this->handle_);
                this->handle_ = InvalidHandleProvider();
            }
        }

      private:
        HANDLE handle_{InvalidHandleProvider()};
    };

    using null_handle = handle<null_handle_value>;
    using ihv_handle = handle<invalid_handle_value>;

    class registry_key
    {
      public:
        registry_key() = default;

        registry_key(const HKEY key)
            : key_(key)
        {
        }

        registry_key(const registry_key&) = delete;
        registry_key& operator=(const registry_key&) = delete;

        registry_key(registry_key&& obj) noexcept
            : registry_key()
        {
            this->operator=(std::move(obj));
        }

        registry_key& operator=(registry_key&& obj) noexcept
        {
            if (this != obj.GetRef())
            {
                this->~registry_key();
                this->key_ = obj.key_;
                obj.key_ = nullptr;
            }

            return *this;
        }

        ~registry_key()
        {
            if (this->key_)
            {
                RegCloseKey(this->key_);
            }
        }

        operator HKEY() const
        {
            return this->key_;
        }

        operator bool() const
        {
            return this->key_ != nullptr;
        }

        HKEY* operator&()
        {
            return &this->key_;
        }

        registry_key* GetRef()
        {
            return this;
        }

        const registry_key* GetRef() const
        {
            return this;
        }

      private:
        HKEY key_{};
    };

    registry_key open_or_create_registry_key(const HKEY base, const std::string& input);

    bool is_wine();
    bool is_shutdown_in_progress();

    __declspec(noreturn) void raise_hard_exception();
    std::string load_resource(int id);

    void relaunch_self();
    __declspec(noreturn) void terminate(uint32_t code = 0);

    std::string get_user_name();
}
