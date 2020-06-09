// Tests.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "gtest/gtest.h"

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    RUN_ALL_TESTS();
    std::getchar(); // keep console window open until Return keystroke
}