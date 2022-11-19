#ifndef KOOLANG_UTIL_THREADPOOL_H
#define KOOLANG_UTIL_THREADPOOL_H

#include "util/debug.h"
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

template <typename Arg> class ThreadPool {
public:
    using FnType = std::function<void(Arg&)>;

    ThreadPool(unsigned int numThreads = std::thread::hardware_concurrency() - 1) {
        numThreads = std::max(numThreads, 1U);

        auto worker = [this]() {
            while (true) {
                std::unique_lock<std::mutex> lock(m_queueMutex);
                m_queueCV.wait(lock, [this]() -> bool { return !m_work.empty() || !m_running; });

                if (m_work.empty() && !m_running) {
                    return;
                }

                Work task = m_work.front();

                m_work.pop();
                lock.unlock();

                task.Func(task.Argument);

                lock.lock();
                m_tasks -= 1;
                m_finishedCV.notify_one();
            }
        };

        m_threads.reserve(numThreads);
        for (unsigned int i = 0; i < numThreads; i++) {
            m_threads.push_back(std::thread(worker));
        }
    }

    void Wait() {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_finishedCV.wait(lock, [this]() -> bool { return m_work.empty() && m_tasks == 0; });
    }

    void Spawn(const FnType& func, const Arg& arg) {
        m_queueMutex.lock();
        m_work.emplace(func, arg);
        m_tasks += 1;
        m_queueMutex.unlock();

        m_queueCV.notify_one();
    }

    void Join() {

        if (!m_running) {
            return;
        }

        m_running = false;
        m_queueCV.notify_all();

        for (auto& worker : m_threads) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }
    ~ThreadPool() { Join(); }

private:
    struct Work {
        FnType Func;
        Arg Argument;
    };

    std::atomic<bool> m_running = true;
    unsigned int m_tasks        = 0;

    std::mutex m_queueMutex;
    std::condition_variable m_queueCV;
    std::condition_variable m_finishedCV;

    std::queue<Work> m_work;
    std::vector<std::thread> m_threads;
};

#endif
