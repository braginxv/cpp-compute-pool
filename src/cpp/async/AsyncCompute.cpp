
#include "AsyncCompute.h"
#include <boost/range/algorithm.hpp>

namespace fibers = boost::fibers;
using namespace std;

AsyncCompute::AsyncCompute(unsigned int concurrency) :
        _concurrency(concurrency),
        _isRunning(false),
        _tasksToDispatch(DISPATCHING_BUFFER_LENGTH) {}

std::future<void> AsyncCompute::run() {
    _initializingMutex.lock();

    if (_isRunning) {
        _initializingMutex.unlock();
        throw runtime_error("It's not possible to start the AsyncCompute twice");
    }

    auto waitCompletion = make_shared<std::promise<void>>();

    createPool([this, waitCompletion] {
        _isRunning = true;
        _initializingMutex.unlock();
        waitCompletion->set_value();
    });

    return waitCompletion->get_future();
}

void AsyncCompute::createPool(std::function<void()> &&onComplete) {
    _threadPool.reserve(_concurrency);

    auto waitTillAllAreInitialized = std::make_shared<boost::fibers::barrier>(_concurrency);

    _threadPool.emplace_back([this, waitTillAllAreInitialized, initComplete {std::move(onComplete)}] {
        if (_concurrency > 1) {
            initContext();
        }

        waitTillAllAreInitialized->wait();
        initComplete();

        for (auto &task: _tasksToDispatch) {
            fibers::fiber(fibers::launch::post, move(task)).detach();
        }

        _shutdown.notify_all();
    });

    for (size_t index = 1; index < _concurrency; ++index) {
        _threadPool.emplace_back([this, waitTillAllAreInitialized] {
            initContext();

            waitTillAllAreInitialized->wait();

            boost::fibers::mutex lockMutex;
            unique_lock<boost::fibers::mutex> waitTermination(lockMutex);

            _shutdown.wait(waitTermination);
        });
    }
}

std::future<void> AsyncCompute::shutdown() {
    _initializingMutex.lock();

    if (!_isRunning) {
        _initializingMutex.unlock();
        throw runtime_error("It's not possible to shutdown AsyncCompute that hasn't been started yet");
    }

    return std::async(launch::async, [this] {
        try {
            _tasksToDispatch.close();
            boost::for_each(_threadPool, [](thread &worker) { worker.join(); });
        } catch(...) {}

        _isRunning = false;
        _initializingMutex.unlock();
    });
}

AsyncCompute::~AsyncCompute() {
    if (_isRunning) {
        boost::fibers::mutex waitMutex;
        unique_lock<boost::fibers::mutex> waitLock(waitMutex);
        shutdown().wait();
    }
}
