#include <gtest/gtest.h>
#include <thread>
#include <boost/range/algorithm.hpp>
#include "AsyncComputeTest.h"

using namespace std::chrono;

TEST_P(ParameterizedComputePool, futureTaskToBeCompleted) {
#define N 100
    std::atomic<uint> counter {N};

    std::vector<boost::fibers::future<void>> asyncTasks;
    for (int index {0}; index < N; ++index) {
        asyncTasks.emplace_back(_computePool.submit(generateTask([&counter] { counter--; })));
    }

    boost::for_each(asyncTasks, [](auto &task) { task.wait(); });

    ASSERT_EQ(0, counter);
}

TEST_P(ParameterizedComputePool, asyncTasksToBeCompleted) {
#define N 100
    std::atomic<uint> counter {N};
    std::condition_variable waitingAllToBeCompleted;

    for (int index {0}; index < N; ++index) {
        _computePool.async(generateTask(), [&counter, &waitingAllToBeCompleted] {
            counter--;
            if (counter == 0) {
                waitingAllToBeCompleted.notify_all();
            }
        });
    }

    std::mutex localMutex;
    std::unique_lock<std::mutex> waitCompletion {localMutex};
    waitingAllToBeCompleted.wait_for(waitCompletion, maxTimeToAwaitAsyncTests);

    ASSERT_EQ(0, counter);
}

TEST_F(ComputePool, yieldControl) {
    std::atomic<uint> notCompletedTasks {N};
    std::atomic<uint> remainingTasksToStart {N};

    std::vector<boost::fibers::future<void>> asyncTasks;
    for (int index {0}; index < N; ++index) {
        asyncTasks.emplace_back(_computePool.submit([&, this] {
            remainingTasksToStart--;
            boost::this_fiber::yield();
            ASSERT_EQ(0, remainingTasksToStart);
            std::this_thread::sleep_for(meanTaskTimeExecution + milliseconds(_dist(_gen)));
            notCompletedTasks--;
        }));
    }

    boost::for_each(asyncTasks, [](auto &task) { task.wait(); });

    ASSERT_EQ(0, notCompletedTasks);
}

TEST_F(ComputePool, notAllTasksHaveBeenStartedWhenUsingThreadSleeps) {
    const uint parallelTasksNumber = std::thread::hardware_concurrency() * 10;
    std::atomic<uint> notCompletedTasks {parallelTasksNumber};
    std::atomic<uint> remainingTasksToStart {parallelTasksNumber};

    std::vector<boost::fibers::future<void>> asyncTasks;
    bool atLeastOneTaskIsNotStartedAfterAnotherCompleted = false;
    for (int index {0}; index < parallelTasksNumber; ++index) {
        asyncTasks.emplace_back(_computePool.submit([&, this] {
            remainingTasksToStart--;
            std::this_thread::sleep_for(meanTaskTimeExecution + milliseconds(_dist(_gen)));
            atLeastOneTaskIsNotStartedAfterAnotherCompleted |= remainingTasksToStart > 0;
            notCompletedTasks--;
        }));
    }

    boost::for_each(asyncTasks, [](auto &task) { task.wait(); });

    ASSERT_TRUE(atLeastOneTaskIsNotStartedAfterAnotherCompleted);
    ASSERT_EQ(0, notCompletedTasks);
}

TEST_F(ComputePool, fiberSymanticSleep) {
    std::atomic<int> notCompletedTasks {N};
    std::atomic<int> remainingTasksToStart {N};

    std::vector<boost::fibers::future<void>> asyncTasks;
    for (int index {0}; index < N; ++index) {
        asyncTasks.emplace_back(_computePool.submit([&, this] {
            remainingTasksToStart--;
            boost::this_fiber::sleep_for(meanTaskTimeExecution + milliseconds(_dist(_gen)));
            ASSERT_EQ(0, remainingTasksToStart);
            notCompletedTasks--;
        }));
    }

    boost::for_each(asyncTasks, [](auto &task) { task.wait(); });

    ASSERT_EQ(0, notCompletedTasks);
}

INSTANTIATE_TEST_SUITE_P(TestComputePoolWithVariousConcurrencies, ParameterizedComputePool,
        ::testing::ValuesIn({1u, 2u, std::thread::hardware_concurrency(), std::thread::hardware_concurrency() * 2}));

void ComputePool::SetUp() {
    _computePool.run().wait();
}

void ComputePool::TearDown() {
    _computePool.shutdown().wait();
}

ComputePool::ComputePool() : ComputePool(1) {
}

ComputePool::ComputePool(uint concurrency) : _computePool(concurrency), _dist(-5, 5), _gen(_r()) {
}

std::function<void()> ComputePool::generateTask() {
    return [this] {
        std::this_thread::sleep_for(meanTaskTimeExecution + milliseconds(_dist(_gen)));
    };
}

std::function<void()> ComputePool::generateTask(std::function<void()> &&payload) {
    return [this, outerPayload {std::forward<std::function<void()>>(payload)}] {
        std::this_thread::sleep_for(meanTaskTimeExecution + milliseconds(_dist(_gen)));
        outerPayload();
    };
}

ParameterizedComputePool::ParameterizedComputePool() : ComputePool(GetParam()) {
}
