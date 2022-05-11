
#include "AsyncCompute.h"
#include <boost/range/algorithm.hpp>

namespace fibers = boost::fibers;
using namespace std;

AsyncCompute::AsyncCompute(unsigned int concurrency) :
        _concurrency(concurrency),
        _isRunning(false),
        _tasksToDispatch(DISPATCHING_BUFFER_LENGTH) {}

shared_ptr<boost::fibers::condition_variable_any> AsyncCompute::run() {
    _initializingMutex.lock();

    if (_isRunning) {
        _initializingMutex.unlock();
        throw runtime_error("It's not possible to start the AsyncCompute twice");
    }

    auto waitCompletion = make_shared<boost::fibers::condition_variable_any>();

    createPool([this, waitCompletion] {
        _isRunning = true;
        _initializingMutex.unlock();
        waitCompletion->notify_all();
    });

    return waitCompletion;
}

void AsyncCompute::createPool(std::function<void()> &&onComplete) {
    _threadPool.reserve(_concurrency);

    auto waitTillAllAreInitialized = std::make_shared<boost::fibers::barrier>(_concurrency);

    _threadPool.emplace_back([this, waitTillAllAreInitialized, initComplete {std::move(onComplete)}] {
        initContext();

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

shared_ptr<boost::fibers::condition_variable_any> AsyncCompute::shutdown() {
    _initializingMutex.lock();

    if (!_isRunning) {
        _initializingMutex.unlock();
        throw runtime_error("It's not possible to shutdown AsyncCompute that hasn't been started yet");
    }

    _tasksToDispatch.close();

    auto waitCompletion = make_shared<boost::fibers::condition_variable_any>();

    thread([this, waitCompletion] {
        try {
            boost::for_each(_threadPool, [](thread &worker) { worker.join(); });
            _isRunning = false;
            _initializingMutex.unlock();
            waitCompletion->notify_all();

        } catch (const std::exception &e) {
            _isRunning = false;
            _initializingMutex.unlock();
            waitCompletion->notify_all();

            throw e;
        }
    }).detach();

    return waitCompletion;
}

AsyncCompute::~AsyncCompute() {
    if (_isRunning) {
        boost::fibers::mutex waitMutex;
        unique_lock<boost::fibers::mutex> waitLock(waitMutex);
        shutdown()->wait(waitLock);
    }
}
