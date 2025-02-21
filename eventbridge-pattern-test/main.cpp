#include <string>
#include <stdio.h>
#include <iostream>

int main(int argc, char *argv[])
{
    // We will accept file paths to expected patterns in JSON format.
    // These patterns will then be validated using sample data.
    std::string pattern = std::string(argv[0]);
    std::cout << pattern;

    return EXIT_SUCCESS;
}