#include <iostream>

#include "async/AsyncCompute.h"
#include <boost/fiber/all.hpp>
#include <iomanip>
#include <boost/thread/future.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/irange.hpp>
#include <chrono>

using namespace std;

void syncPrint(const string &message) {
    static mutex printMutex;
    lock_guard<mutex> sync(printMutex);

    cout << message << endl;
}

void simulateWork(const std::string &whatWork, int elapsedTimeInMs) {
    syncPrint((stringstream() << "start heavy work: \"" << whatWork << "\"").str());
    std::this_thread::sleep_for(std::chrono::milliseconds(elapsedTimeInMs));
    syncPrint((stringstream() << "end heavy work: \"" << whatWork << "\"").str());
}

void printThread(const std::string &codeName) {
    using namespace std;

    syncPrint((stringstream() << "the code \"" << codeName
                              << "\" is being executed in thread [0x" << hex << this_thread::get_id() << "]").str());
}

static const uint32_t CONCURRENCY = thread::hardware_concurrency();

int main() {
    printThread("start experiment");

    AsyncCompute compute(CONCURRENCY);
    boost::fibers::mutex waitMutex;
    compute.run().wait();

    chrono::steady_clock::time_point t = chrono::steady_clock::now();
    vector<boost::fibers::future<uint32_t>> futureResults;
    boost::for_each(boost::irange(CONCURRENCY), [&](uint32_t index) {
        futureResults.emplace_back(compute.submit<uint32_t>([index] {
            printThread((stringstream() << "start load " << dec << index).str());

            boost::this_fiber::yield();

            printThread((stringstream() << "continue load " << dec << index).str());
            simulateWork((stringstream() << "load " << dec << index).str(), 500);
            return index;
        }));
    });

    boost::for_each(futureResults, [](auto &resultInFuture) {
        auto result = resultInFuture.get();
        cout << "Task #" << dec << result << " is completed on thread 0x" << hex << std::this_thread::get_id() << endl;
    });

    chrono::microseconds us = chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - t);
    syncPrint((stringstream() << "elapsed time " << dec << us.count() << "us").str());

    compute.shutdown().wait();
    printThread("End experiment");
    return 0;
}

