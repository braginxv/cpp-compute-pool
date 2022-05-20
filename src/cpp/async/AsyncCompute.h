#pragma once

#include <thread>
#include <boost/fiber/all.hpp>

class AsyncCompute {
public:
    explicit AsyncCompute(unsigned int concurrency = std::thread::hardware_concurrency());

    std::future<void> run();

    std::future<void> shutdown();

    template<typename T>
    boost::fibers::future<T> submit(std::function<T()> &&task) {
        auto promiseResult = std::make_shared<boost::fibers::promise<T>>();
        boost::fibers::future<T> result = promiseResult->get_future();
        _tasksToDispatch.push([promiseResult, heavyTask{std::forward<std::function<T()>>(task)}] {
            promiseResult->set_value(heavyTask());
        });

        return std::move(result);
    }

    template<typename T>
    void async(std::function<T()> &&task, const std::function<void(T)> &watchResult) {
        _tasksToDispatch.push([&watchResult, heavyTask{std::forward<std::function<T()>>(task)}] {
            watchResult(heavyTask());
        });
    }

    virtual ~AsyncCompute();

private:
#define DISPATCHING_BUFFER_LENGTH 0x1000
    unsigned int _concurrency;
    bool _isRunning;
    std::mutex _initializingMutex;
    std::vector<std::thread> _threadPool;
    boost::fibers::condition_variable _shutdown;

    boost::fibers::buffered_channel<std::function<void()>> _tasksToDispatch;

    void createPool(std::function<void()> &&onComplete);

    void initContext() {
        boost::fibers::use_scheduling_algorithm<boost::fibers::algo::work_stealing>(_concurrency);
    }
};
