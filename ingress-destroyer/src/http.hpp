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

    void makeRequest(const std::string &url, const std::string &method, const std::string *payload = nullptr)
    {
        requestCount++;

        auto handle = curl_easy_init();
        if (!handle)
        {
            throw std::runtime_error("HTTP setup failed");
        }

        curl_easy_setopt(handle, CURLOPT_CAINFO, this->config.caPath.c_str());
        curl_easy_setopt(handle, CURLOPT_SSLCERT, this->config.certPath.c_str());
        curl_easy_setopt(handle, CURLOPT_SSLKEY, this->config.certKeyPath.c_str());
        curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
        curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, strToUpper(method).c_str());
        curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 1);

        if (payload != nullptr)
        {
            curl_easy_setopt(handle, CURLOPT_POSTFIELDS, payload->c_str());
            curl_easy_setopt(handle, CURLOPT_POSTFIELDSIZE, payload->size());
        }

        size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp);

        const auto res = curl_easy_perform(handle);

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
        const std::string payload = R"({"version":"1.0.0","scheduled":false,"channel":"MOBILE","product":"SOUNDS","type":"MOBILE_ALERT","collapse_key":"6493f88c-1b12-56d2-bf04-9521f92fe62b","data":{"cid":"Cid156350s20s","URL":"Url15615593d0020"},"notification":{"alert":{"body":"Body15615593500201","title":"Title1561559350020"},"badge":"2","image":null,"interaction":{"category":"category"},"sound":"Sound1561559350s020","video":{"vpid":"p05fchjx"},"notificationTag":"6493f88c-1b12-56d2-bf04-9521f92fe62b"},"user_id":"510bc8e6-d91a-448c-b2e2-16cdc30f72d3","devices":["IOS","ANDROID","AMAZON"]})";

        this->makeRequest(url, "POST", &payload);
    }
};