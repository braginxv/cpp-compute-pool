
#include <gtest/gtest.h>
#include <thread>
#include "AsyncComputeTest.h"


TEST_F(ComputePool, allTaskToBeCompleted) {
#define N 100
//
//    std::condition_variable_any waitCondition;
//    std::atomic<int> counter {N};
//    std::minstd_rand0 gen;
//    std::random_device r;
//    std::uniform_int_distribution<int> dist(-200, 200);
//
//    for (int index = 0; index < N; ++index) {
//        _computePool.submit([&] {
//            using namespace std::chrono;
//            std::this_thread::sleep_for(500ms + milliseconds(dist(r)))
//        });
//    }
//
//    std::unique_lock<std::mutex>();
//    waitCondition.wait()
//
//    ASSERT_TRUE(counter == 0);
}

TEST_F(ComputePool, secondTest) {
    ASSERT_TRUE(true);
}

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

