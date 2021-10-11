#pragma once
#include <cstdint>

/*****************************************************************************
 *
 ****************************************************************************/

struct dimensions {
	uint32_t width{};
	uint32_t height{};
};

/*****************************************************************************
 *
 ****************************************************************************/

class canvas {
public:
	virtual ~canvas() = default;
	virtual dimensions get_dimensions() const = 0;
	virtual void paint(const void* image) = 0;
};
