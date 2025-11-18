#include <iostream>
#include <csignal>
#include <cstdlib>
#include <string>

#include "./src/config.hpp"
#include "./src/http.hpp"
#include "./src/threadManager.hpp"

using namespace std::chrono_literals;

volatile std::sig_atomic_t signal_flag = 0;

void SignalHandler(int signal)
{
    if (signal == SIGINT || signal == SIGTERM)
    {
        signal_flag = 1;
    }
}

int main(int argc, char *argv[])
{
    std::signal(SIGINT, SignalHandler);  // Handle Ctrl+C (SIGINT)
    std::signal(SIGTERM, SignalHandler); // Handle termination request (SIGTERM)

    const auto config = Config::LoadConfig();

    std::cout << "Welcome to Ingress Destroyer" << std::endl;
    std::cout << "Endpoint: " << config.endpoint << std::endl;

    ThreadManager::GetInstance().Start();

    // Blocking call to keep the process running until a signal is sent.
    while (!signal_flag)
    {
        // Start thread manager to make requests to the endpoint specified.
        ThreadManager::GetInstance()
            .AddTaskToThread([&]
                             {
                                 // We will make requests to the endpoint as many times as possible.
                                 //  Http &http = Http::GetInstance(config);
                                 //  http.POST("/notifications");
                                 // We do not care what the response is. We only want to make lots of requests.
                             });

        std::this_thread::sleep_for(500ms);
    }

    ThreadManager::GetInstance().Stop();

    std::cout << std::endl
              << "Application Killed"
              << std::endl;
}
