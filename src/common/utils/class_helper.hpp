#pragma once

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
