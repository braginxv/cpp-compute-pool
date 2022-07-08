#pragma once

#include "async/AsyncCompute.h"
#include <gtest/gtest.h>
#include <chrono>

class ComputePool : public ::testing::Test {
protected:
    ComputePool();

    explicit ComputePool(uint32_t);

    void SetUp() override;

    void TearDown() override;

    std::function<void()> generateTask();

    std::function<void()> generateTask(std::function<void()> &&);

    static constexpr std::chrono::duration<long> maxTimeToAwaitAsyncTests {std::chrono::seconds(20)};

    static constexpr std::chrono::duration<long, std::ratio<1, 1000>> meanTaskTimeExecution {std::chrono::milliseconds(10)};

    AsyncCompute _computePool;
    std::random_device _r;
    std::minstd_rand0 _gen;
    std::uniform_int_distribution<long> _dist;
};

class ParameterizedComputePool : public ComputePool, public ::testing::WithParamInterface<uint32_t> {
protected:
    ParameterizedComputePool();
};

constexpr std::chrono::duration<long> ComputePool::maxTimeToAwaitAsyncTests;
constexpr std::chrono::duration<long, std::ratio<1, 1000>> ComputePool::meanTaskTimeExecution;
