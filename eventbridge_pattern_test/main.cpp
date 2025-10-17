#include <string>
#include <stdio.h>
#include <iostream>

int main(int argc, char *argv[])
{
    // We will accept file paths to expected patterns in JSON format.
    // These patterns will then be validated using sample data.
    if (argc < 3)
    {
        std::cerr << "You must provide pattern file path and test event" << std::endl;
        return EXIT_FAILURE;
    }

    std::string pattern = std::string(argv[1]);
    std::cout << pattern << std::endl;
    std::this_thread::

        return EXIT_SUCCESS;
}