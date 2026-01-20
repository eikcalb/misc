#pragma once
#include <string>
#include <unordered_map>
#include <fstream>

class Utils
{
public:
    const static void parseINI(const std::string &path, std::unordered_map<std::string, std::string> &out)
    {
        // Retrieve file contents and populate the output vector.
        std::ifstream file(path);

        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open file: " + path);
        }

        std::string line;
        while (std::getline(file, line))
        {
            // Skip empty lines and comments
            if (line.empty() || line[0] == ';' || line[0] == '#')
            {
                continue;
            }

            // Find the '=' delimiter
            size_t delim = line.find('=');
            if (delim != std::string::npos)
            {
                std::string key = line.substr(0, delim);
                std::string value = line.substr(delim + 1);
                std::cout << value;
                out[key] = value;
            }

            line.clear();
        }

        file.close();
    };
};