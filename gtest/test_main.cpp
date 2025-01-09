#include <gtest/gtest.h>

#include "TypePaser.h"

template<class T>
bool SecrchVect(const std::vector<T>& v,const T& str)
{
    return (std::find(v.begin(), v.end(), str) != v.end());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}