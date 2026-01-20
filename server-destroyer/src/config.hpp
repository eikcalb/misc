#pragma once
#include <iostream>
#include <vector>
#include <string>

#include "./utils.hpp"

const char *CA_PATH_ENV = "CA";
const char *CERT_PATH_ENV = "CERT";
const char *CERT_KEY_PATH_ENV = "CERT_KEY";
const char *COOKIE_ENV = "COOKIE";
const char *ENDPOINT_ENV = "ENDPOINT";
const char *HEADERS_ENV = "HEADERS";
const char *METHOD_ENV = "METHOD";
const char *PATH_ENV = "PATH";
const char *PAYLOAD_ENV = "PAYLOAD";

struct Config
{
    std::string caPath;
    std::string certPath;
    std::string certKeyPath;
    std::string cookie;
    std::string endpoint;
    std::string method;
    std::string path;
    std::string payload;

    std::vector<std::string> headers;

private:
    std::string getEnv(const char *key, std::string defValue = "")
    {
        const auto env_ptr = std::getenv(key);
        if (env_ptr == nullptr)
        {
            if (defValue != "")
            {
                return defValue;
            }

            std::cout << "Environment variable (\"" << key << "\") not found" << std::endl;
            exit(1);
        }

        if (strlen(env_ptr) < 1)
        {
            // If the environment variable exists, bt it's empty, we ignore.
            std::cout << "Environment variable (\"" << key << "\") is empty" << std::endl;
            return "";
        }

        return std::string(env_ptr);
    }

    void parseHeaders(const std::string &csvHeaders)
    {
        std::string header;
        const char delimiter = ',';

        size_t pos_start = 0, pos_end;
        std::string token;

        headers.clear();

        while ((pos_end = csvHeaders.find(delimiter, pos_start)) != std::string::npos)
        {
            token = csvHeaders.substr(pos_start, pos_end - pos_start);
            pos_start = pos_end + sizeof(delimiter);
            headers.emplace_back(token);
        }

        headers.emplace_back(csvHeaders.substr(pos_start));
    }

public:
    [[]] static Config LoadConfig()
    {
        Config c;
        c.caPath = c.getEnv(CA_PATH_ENV);
        c.certPath = c.getEnv(CERT_PATH_ENV);
        c.certKeyPath = c.getEnv(CERT_KEY_PATH_ENV);
        c.endpoint = c.getEnv(ENDPOINT_ENV);
        c.method = c.getEnv(METHOD_ENV);
        c.path = c.getEnv(PATH_ENV);
        c.payload = c.getEnv(PAYLOAD_ENV, "");

        const auto rawHeaders = c.getEnv(HEADERS_ENV, "");
        c.parseHeaders(rawHeaders);

        return c;
    }

    static Config LoadConfig(const char *filePath)
    {
        Config c;

        // Read the ini file.
        std::unordered_map<std::string, std::string> vars;
        Utils::parseINI(filePath, vars);

        c.caPath = vars.find(CA_PATH_ENV) != vars.end() ? vars.at(CA_PATH_ENV) : c.getEnv(CA_PATH_ENV);
        c.certPath = vars.find(CERT_PATH_ENV) != vars.end() ? vars.at(CERT_PATH_ENV) : c.getEnv(CERT_PATH_ENV);
        c.certKeyPath = vars.find(CERT_KEY_PATH_ENV) != vars.end() ? vars.at(CERT_KEY_PATH_ENV) : c.getEnv(CERT_KEY_PATH_ENV);
        c.cookie = vars.find(COOKIE_ENV) != vars.end() ? vars.at(COOKIE_ENV) : c.getEnv(COOKIE_ENV);
        c.endpoint = vars.find(ENDPOINT_ENV) != vars.end() ? vars.at(ENDPOINT_ENV) : c.getEnv(ENDPOINT_ENV);
        c.method = vars.find(METHOD_ENV) != vars.end() ? vars.at(METHOD_ENV) : c.getEnv(METHOD_ENV);
        c.path = vars.find(PATH_ENV) != vars.end() ? vars.at(PATH_ENV) : c.getEnv(PATH_ENV);
        c.payload = vars.find(PAYLOAD_ENV) != vars.end() ? vars.at(PAYLOAD_ENV) : c.getEnv(PAYLOAD_ENV);

        const auto rawHeaders = vars.find(HEADERS_ENV) != vars.end() ? vars.at(HEADERS_ENV) : c.getEnv(HEADERS_ENV, "");
        c.parseHeaders(rawHeaders);

        return c;
    }
};