#pragma once

// C/C++
#include <array>

#include "stconsts.h"

typedef std::array<float, cArrayLength> Grid;
class World
{
public:
	std::array<float, cArrayLength> waterHeight;
	std::array<float, cArrayLength> horVelocity;
	std::array<float, cArrayLength> vertVelocity;

	World(void);
	~World(void);
};

