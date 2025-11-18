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
    Config config;
    std::atomic<unsigned long> requestCount;

    Http(const Config config) : config(config)
    {
        requestCount.store(0);

        if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK)
        {
            throw std::runtime_error("HTTP setup failed");
        }
    }

    ~Http()
    {

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

    void makeRequest(const std::string url, const std::string method, const std::string *payload = nullptr)
    {
        requestCount++;

        auto handle = curl_easy_init();
        if (!handle)
        {
            throw std::runtime_error("HTTP setup failed");
        }

        struct curl_slist *list = nullptr;

        curl_easy_setopt(handle, CURLOPT_CAINFO, this->config.caPath.c_str());
        curl_easy_setopt(handle, CURLOPT_SSLCERT, this->config.certPath.c_str());
        curl_easy_setopt(handle, CURLOPT_SSLKEY, this->config.certKeyPath.c_str());
        curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
        curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, strToUpper(method).c_str());
        curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 1);
        curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 1);

        if (payload != nullptr)
        {
            list = curl_slist_append(list, "Content-Type: application/json");

            curl_easy_setopt(handle, CURLOPT_POSTFIELDS, payload->c_str());
            curl_easy_setopt(handle, CURLOPT_POSTFIELDSIZE, payload->size());
        }

        curl_easy_setopt(handle, CURLOPT_HTTPHEADER, list);
        curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, discard);

        const auto res = curl_easy_perform(handle);
        curl_slist_free_all(list);

        if (res != CURLE_OK)
        {
            std::cout << "❌ Request failed: " << curl_easy_strerror(res) << std::endl;
            curl_easy_cleanup(handle);

            throw std::runtime_error("Failed to make network request");
        }

        curl_easy_cleanup(handle);

        uint httpCode = 0;
        curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &httpCode);

        if (httpCode < 200 || httpCode >= 300)
        {
            std::cout << "🥀";
        }
        else
        {
            std::cout << "👌";
        }
    }

    static size_t discard(char *, size_t size, size_t nmemb, void *userdata)
    {
        return size * nmemb;
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

    void POST(const std::string &path)
    {
        const auto &url = this->config.endpoint + path;

        this->makeRequest(url, "POST", &this->config.payload);
    }
};
