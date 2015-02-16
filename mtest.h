#pragma once
#include <iostream>


class mtest
{
public:
	mtest();
public:
	void operator ()(const char *msg, ...);
};
