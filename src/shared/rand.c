#include <DiepDesktop/shared/rand.h>
#include <DiepDesktop/shared/debug.h>

#include <math.h>


Static uint32_t RandomSeed;


void
SeedRand(
	uint32_t Seed
	)
{
	RandomSeed = Seed;
}


uint32_t
Rand(
	void
	)
{
	return RandomSeed = (1103515245 * RandomSeed + 12345) & 0x7FFFFFFF;
}


float
RandF(
	void
	)
{
	return (double) Rand() / (double) 0x7FFFFFFF;
}


uint32_t
RandBit(
	void
	)
{
	return Rand() & 64;
}


float
RandAngle(
	void
	)
{
	return (RandF() - 0.5) * M_PI * 2.0;
}
