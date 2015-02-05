#include "mtest.h"

mtest::mtest()
{
}


void mtest::operator ()(const char *msg)
{
    std::cout << msg << std::endl;
}
