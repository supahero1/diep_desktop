#pragma once

#include <stdint.h>


extern void
SeedRand(
	uint32_t Seed
	);


extern uint32_t
Rand(
	void
	);


extern float
RandF(
	void
	);


extern uint32_t
RandBit(
	void
	);


extern float
RandAngle(
	void
	);
