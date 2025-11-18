#pragma once
#include <algorithm>
#include <atomic>
#include <iostream>
#include <memory>
#include <string>

#include <curl/curl.h>

#include "./config.hpp"

class Http
{
private:
    std::string endpoint;
    std::atomic<unsigned long> requestCount;
    std::unique_ptr<CURL *> defaultHandle;

    Http(const Config config)
    {
        endpoint = config.endpoint;

        requestCount.store(0);

        curl_global_init(CURL_GLOBAL_ALL);

        *defaultHandle = curl_easy_init();

        curl_easy_setopt(defaultHandle.get(), CURLOPT_CAINFO, config.caPath.c_str());
        curl_easy_setopt(defaultHandle.get(), CURLOPT_SSLCERT, config.certPath.c_str());
        curl_easy_setopt(defaultHandle.get(), CURLOPT_SSLKEY, config.certKeyPath.c_str());
    }

    ~Http()
    {
        curl_easy_cleanup(defaultHandle.get());

        curl_global_cleanup();
    }

    std::string strToUpper(std::string s)
    {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c)
                       { return std::toupper(c); } // correct
        );
        return s;
    }

    void makeRequest(const std::string &url, const std::string &method, const std::string *payload = nullptr)
    {
        requestCount++;

        std::cout << url << std::endl;

        const auto handle = curl_easy_duphandle(defaultHandle.get());

        curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
        curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, strToUpper(method).c_str());
        curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 0);

        if (payload != nullptr)
        {
            curl_easy_setopt(handle, CURLOPT_POSTFIELDS, payload->c_str());
            curl_easy_setopt(handle, CURLOPT_POSTFIELDSIZE, payload->size());
        }

        size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp);

        const auto res = curl_easy_perform(handle);
        curl_easy_cleanup(handle);

        if (res != CURLE_OK)
        {
            std::cout << "❌ Request failed" << std::endl;
            throw std::runtime_error("Failed to make network request");
        }
    }

public:
    static Http &GetInstance(const Config &c)
    {
        static Http instance(c);
        return instance;
    }

    unsigned long GetRequestCount()
    {
        return this->requestCount.load();
    }

    void POST(std::string path)
    {
        const auto url = this->endpoint + path;
        this->makeRequest(url, "POST");
    }
};