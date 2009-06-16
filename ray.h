#pragma once

struct Ray 
{
	__m128 positionX, positionY, positionZ;
	__m128 directionX, directionY, directionZ;
};