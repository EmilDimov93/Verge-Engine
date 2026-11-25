// Copyright 2025 Emil Dimov
// Licensed under the Apache License, Version 2.0

#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace VE
{
    class ThreadPool
    {
    public:
        explicit ThreadPool(uint32_t workerCount)
        {
            for (uint32_t workerIndex = 0; workerIndex < workerCount; ++workerIndex)
                workerThreads.emplace_back([this, workerIndex](std::stop_token stopToken)
                                           { loop(stopToken, workerIndex); });
        }

        void submit(std::function<void(uint32_t workerIndex)> job)
        {
            {
                std::lock_guard lock(queueMutex);
                pendingJobs.push(std::move(job));
                ++unfinishedJobCount;
            }
            queueCondition.notify_all();
        }

        void wait()
        {
            std::unique_lock lock(queueMutex);
            queueCondition.wait(lock, [this]
                                { return unfinishedJobCount == 0; });
        }

    private:
        void loop(std::stop_token stopToken, uint32_t workerIndex)
        {
            std::unique_lock lock(queueMutex);
            while (queueCondition.wait(lock, stopToken, [this]
                                       { return !pendingJobs.empty(); }))
            {
                std::function<void(uint32_t workerIndex)> job = std::move(pendingJobs.front());
                pendingJobs.pop();

                lock.unlock();
                job(workerIndex);
                lock.lock();

                if (--unfinishedJobCount == 0)
                    queueCondition.notify_all();
            }
        }

        std::mutex queueMutex;
        std::condition_variable_any queueCondition;
        std::queue<std::function<void(uint32_t workerIndex)>> pendingJobs;
        uint32_t unfinishedJobCount = 0;
        std::vector<std::jthread> workerThreads;
    };
}