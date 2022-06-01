#include <gtest/gtest.h>
#include <thread>
#include <boost/range/algorithm.hpp>
#include "AsyncComputeTest.h"

using namespace std::chrono;

TEST_P(ComputePool, futureTaskToBeCompleted) {
#define N 100
    std::atomic<int> counter{N};

    std::vector<boost::fibers::future<void>> asyncTasks;
    for (int index{0}; index < N; ++index) {
        asyncTasks.emplace_back(_computePool.submit(generateTask([&counter] { counter--; })));
    }

    boost::for_each(asyncTasks, [](auto &task) { task.wait(); });

    ASSERT_EQ(0, counter);
}

TEST_P(ComputePool, asyncTasksToBeCompleted) {
#define N 100
    std::atomic<int> counter{N};
    std::condition_variable waitingAllToBeCompleted;

    for (int index{0}; index < N; ++index) {
        _computePool.async(generateTask(), [&counter, &waitingAllToBeCompleted] {
            counter--;
            if (counter == 0) {
                waitingAllToBeCompleted.notify_all();
            }
        });
    }

    std::mutex localMutex;
    std::unique_lock<std::mutex> waitCompletion{localMutex};
    waitingAllToBeCompleted.wait_for(waitCompletion, maxTimeToAwaitAsyncTests);

    ASSERT_EQ(0, counter);
}

TEST_F(ComputePool, yieldControl) {
    std::atomic<int> notCompletedTasks{N};
    std::atomic<int> remainingTasksToStart{N};

    std::vector<boost::fibers::future<void>> asyncTasks;
    for (int index{0}; index < N; ++index) {
        asyncTasks.emplace_back(_computePool.submit([&, this] {
            remainingTasksToStart--;
            boost::this_fiber::yield();
            ASSERT_EQ(0, remainingTasksToStart);
            std::this_thread::sleep_for(100ms + milliseconds(_dist(_gen)));
            notCompletedTasks--;
        }));
    }

    boost::for_each(asyncTasks, [](auto &task) { task.wait(); });

    ASSERT_EQ(0, notCompletedTasks);
}

INSTANTIATE_TEST_SUITE_P(PoolVariation, ComputePool,
        ::testing::ValuesIn({1u, 2u, std::thread::hardware_concurrency(), std::thread::hardware_concurrency() * 2}));

void ComputePool::SetUp() {
    syncPrint("SetUp call");

    _computePool.run().wait();
}

void ComputePool::TearDown() {
    syncPrint("TearDown call");

    _computePool.shutdown().wait();
}

void ComputePool::syncPrint(const std::string &message) {
    static std::mutex printMutex;
    std::lock_guard<std::mutex> sync(printMutex);

    std::cout << (message) << std::endl;
}

ComputePool::ComputePool() :
_computePool(GetParam()), _dist(-5, 5), _gen(_r()) {
}

std::function<void()> ComputePool::generateTask() {
    return [this] {
        std::this_thread::sleep_for(10ms + milliseconds(_dist(_gen)));
    };
}

std::function<void()> ComputePool::generateTask(std::function<void()> &&payload) {
    return [this, outerPayload{std::forward<std::function<void()>>(payload)}] {
        std::this_thread::sleep_for(10ms + milliseconds(_dist(_gen)));
        outerPayload();
    };
}

const std::chrono::duration<int> ComputePool::maxTimeToAwaitAsyncTests = 20s;
