#pragma once
#include <iostream>
#include <string>

const char *CA_PATH_ENV = "CA";
const char *CERT_PATH_ENV = "CERT";
const char *CERT_KEY_PATH_ENV = "CERT_KEY";
const char *ENDPOINT_ENV = "ENDPOINT";
const char *PAYLOAD_ENV = "PAYLOAD";

struct Config
{
    std::string caPath;
    std::string certPath;
    std::string certKeyPath;
    std::string endpoint;
    std::string payload;

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
            if (defValue != "")
            {
                return defValue;
            }

            std::cout << "Environment variable (\"" << key << "\") is empty" << std::endl;
            exit(1);
        }

        return std::string(env_ptr);
    }

public:
    static Config LoadConfig()
    {
        Config c;
        c.caPath = c.getEnv(CA_PATH_ENV);
        c.certPath = c.getEnv(CERT_PATH_ENV);
        c.certKeyPath = c.getEnv(CERT_KEY_PATH_ENV);
        c.endpoint = c.getEnv(ENDPOINT_ENV);
        c.payload = c.getEnv(PAYLOAD_ENV);

        return c;
    }
};