#include <iostream>
#include <thread>
#include <tuple>  // Required for std::tuple
#include <string>
#include <cstring> // Required for strcmp
#include <stdexcept> // For handling stoi exceptions

std::tuple<int, std::string> getLvalue(int argc, char *argv[]) {
    std::string l_value;
    std::string p_value;

    // Loop through command-line arguments
    for (int i = 1; i < argc; i++) {
        // Check for the -l flag and get the next argument as the value
        if (strcmp(argv[i], "-l") == 0 && i + 1 < argc) {
            l_value = argv[i + 1];
            i++; // Skip the value part in the next iteration
        }
        // Check for the -p flag and get the next argument as the value
        else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            p_value = argv[i + 1];
            i++; // Skip the value part in the next iteration
        }
    }

    // Ensure that l_value and p_value are not empty
    if (l_value.empty()) {
        throw std::invalid_argument("Missing or invalid -l flag");
    }
    if (p_value.empty()) {
        throw std::invalid_argument("Missing or invalid -p flag");
    }

    // Convert l_value to integer safely
    int l_int;
    try {
        l_int = std::stoi(l_value);
    } catch (const std::invalid_argument& e) {
        throw std::invalid_argument("Invalid value for -l, must be an integer");
    } catch (const std::out_of_range& e) {
        throw std::out_of_range("Value for -l is out of range");
    }

    std::tuple<int, std::string> return_value = std::make_tuple(l_int, p_value);
    return return_value;
}

int main(int argc, char *argv[]) {
   
    std::tuple<int, std::string> values = getLvalue(argc, argv);

    int listen_port = std::get<0>(values);
    std::string search_pattern = std::get<1>(values);

    

    return 0; // Return 0 for successful execution
}
