package com.goeswhere.tracer;

public class Ray
{
	SSEFloat positionX, positionY, positionZ;
	SSEFloat directionX, directionY, directionZ;
	@Override public String toString() {
		return "Ray [positionX=" + positionX + ", positionY=" + positionY + ", positionZ=" + positionZ
				+ ", directionX=" + directionX + ", directionY=" + directionY + ", directionZ=" + directionZ + "]";
	}
}
