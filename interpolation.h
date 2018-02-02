#pragma once

template <class A>
A Interpolate(A a, A b, double distance)
{
	A temp;
	temp = (A)((a*(1.0-distance)) + (b*distance));
	return temp;
}
