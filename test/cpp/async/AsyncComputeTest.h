#pragma once

#include "async/AsyncCompute.h"
#include <gtest/gtest.h>

class ComputePool: public ::testing::Test {
protected:
    void SetUp() override;

    void TearDown() override;

    static void syncPrint(const std::string &message);

    AsyncCompute _computePool;
};
