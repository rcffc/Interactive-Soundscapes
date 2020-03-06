#pragma once

#include <string>

class Element 
{
	public:

		Element(std::string aJoint, float aX, float aY, float aZ);
		std::string joint;
		float x;
		float y;
		float z;
};