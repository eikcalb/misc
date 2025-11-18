#pragma once
#include <condition_variable>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <thread>
#include <vector>

typedef std::function<void()> Task;

typedef std::thread Thread;

/**
 * Manages a pool of threads and dispatches work to threads available for work.
 */
class ThreadManager
{
public:
    static ThreadManager &GetInstance()
    {
        static ThreadManager instance;
        return instance;
    }

    ~ThreadManager()
    {
        this->Stop();
    }

    // Add a task to the thread pool.
    void AddTaskToThread(Task task)
    {
        {
            std::lock_guard<std::mutex> lock(tasksMutex);
            tasks.emplace_back(std::move(task));
        }

        condition.notify_all();
    }

    void Start()
    {
        const auto concurrencyCount = getConcurrencyCount();
        std::cout << "Starting: " << concurrencyCount << "threads" << std::endl;

        // Initialize the thread pool.
        for (uint i = 0; i < concurrencyCount; i++)
        {
            threads.emplace_back(std::bind(&ThreadManager::threadFunction, this, i));
        }
    }

    void Stop()
    {
        std::cout << "Stopping threads" << std::endl;

        isRunning.store(false);
        condition.notify_all();

        for (Thread &thread : threads)
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }
    }

private:
    std::vector<Thread> threads;
    std::deque<Task> tasks;
    std::condition_variable condition;
    std::atomic<bool> isRunning;

    std::mutex tasksMutex;

    ThreadManager()
    {
        isRunning.store(true);
    }

    // Delete copy constructor.
    ThreadManager(const ThreadManager &) = delete;
    // Delete move constructor.
    ThreadManager(ThreadManager &&other) = delete;
    // Prevent assignment.
    ThreadManager &operator=(const ThreadManager &) = delete;

    unsigned int getConcurrencyCount()
    {
        const auto concurrency = std::thread::hardware_concurrency();

        return concurrency > 0 ? concurrency : 1;
    }

    /**
     * Function executed by each thread in the pool.
     *
     * It will wait for available tasks and run the tasks in FIFO order.
     */
    void threadFunction(int id)
    {
        std::stringstream message;
        message << "Starting thread: " << id;
        message << "(" << std::this_thread::get_id() << ")" << std::endl;
        std::cout << message.str();

        // While the thread is running, we need to pause the thread without consuming resources.
        while (isRunning.load())
        {
            Task task;
            {

                std::unique_lock<std::mutex> lock(tasksMutex);

                // Wait for a task if the task queue is empty.
                condition.wait(lock, [this]
                               {
                    // If we have a signal to exit, we should stop waiting.
                    if (!this->isRunning.load()) {
                        return true;
                    }

                    // When a task exists in the queue, this will return true and exit.
                    return !this->tasks.empty(); });

                if (!isRunning.load() && tasks.empty())
                {
                    continue;
                }

                // Get the next task from the task queue.
                task = std::move(tasks.front());
                tasks.pop_front();

                lock.unlock();
                condition.notify_all();
            }

            // Execute the task
            if (task)
            {
                try
                {
                    task();
                }
                catch (std::exception &ex)
                {
                    std::cerr << ex.what() << std::endl;
                }
            }
        }

        // This will trigger all pending threads waiting when the loop
        // has been triggered to stop.
        condition.notify_all();
    }
};