#pragma once

#include "async/AsyncCompute.h"
#include <gtest/gtest.h>
#include <chrono>

class ComputePool: public ::testing::TestWithParam<uint> {
protected:
    ComputePool();

    void SetUp() override;

    void TearDown() override;

    static void syncPrint(const std::string &message);

    std::function<void()> generateTask();

    std::function<void()> generateTask(std::function<void()> &&);

    static const std::chrono::duration<int> maxTimeToAwaitAsyncTests;

    AsyncCompute _computePool;
    std::random_device _r;
    std::minstd_rand0 _gen;
    std::uniform_int_distribution<int> _dist;
};
