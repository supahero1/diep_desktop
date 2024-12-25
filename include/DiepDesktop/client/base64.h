#pragma once

#include <stdint.h>


extern uint8_t*
Base64Encode(
	const uint8_t* Data,
	uint64_t InLength,
	uint64_t* OutLength
	);


extern uint8_t*
Base64Decode(
	const uint8_t* Data,
	uint64_t InLength,
	uint64_t* OutLength
	);
