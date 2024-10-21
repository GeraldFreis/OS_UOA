#include <iostream>
#include <thread>
#include <tuple>
#include <string>

std::tuple<int, std::string> getLvalue(int argc, char *argv[]){
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
    std::tuple<int, std::string> return_value = std::make_tuple(stoi(l_value), p_value);
    return return_value; 
}

int main(int argc, char *argv[])
{
    std::tuple<int, std::string> values = getLvalue(argc, argv);
    int listen_port = std::get<0>(values);
    std::string search_pattern = std::get<1>(values);
    std::cout << listen_port << "\n";
}
