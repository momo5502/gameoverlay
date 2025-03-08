#pragma once

#include <string_view>

#include "gl_object.hpp"

gl_object create_program(std::string_view vertex, std::string_view fragment);
