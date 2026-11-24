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
                workerThreads.emplace_back([this](std::stop_token stopToken)
                                           { loop(stopToken); });
        }

        void submit(std::function<void()> job)
        {
            {
                std::lock_guard lock(queueMutex);
                pendingJobs.push(std::move(job));
                ++unfinishedJobCount;
            }
            queueCondition.notify_all();
        }

    private:
        void loop(std::stop_token stopToken)
        {
            std::unique_lock lock(queueMutex);
            while (queueCondition.wait(lock, stopToken, [this]
                                       { return !pendingJobs.empty(); }))
            {
                std::function<void()> job = std::move(pendingJobs.front());
                pendingJobs.pop();

                lock.unlock();
                job();
                lock.lock();

                if (--unfinishedJobCount == 0)
                    queueCondition.notify_all();
            }
        }

        std::mutex queueMutex;
        std::condition_variable_any queueCondition;
        std::queue<std::function<void()>> pendingJobs;
        uint32_t unfinishedJobCount = 0;
        std::vector<std::jthread> workerThreads;
    };
}