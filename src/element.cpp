#pragma once
#include "element.h"
#include <string>


Element::Element(std::string aJoint, float aX, float aY, float aZ) {
	joint = aJoint;
	x = aX;
	y = aY;
	z = aZ;
}
