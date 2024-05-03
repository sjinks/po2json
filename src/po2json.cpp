#include <cstdlib>
#include <fstream>
#include <iostream>
#include "gettext.h"
#include "processor.h"

int main(int argc, char** argv)
{
    const std::span<char*> args(argv, argc);
    if (args.size() != 3) {
        std::cerr << "Usage: " << args[0] << " <po-file> <json-file>\n";
        return EXIT_FAILURE;
    }

    try {
        const po::file po(args[1]);
        std::ofstream json(args[2], std::ios::out | std::ios::trunc);
        if (!json) {
            std::cerr << "Failed to create " << args[2] << '\n';
            return EXIT_FAILURE;
        }

        json.exceptions(std::ios::failbit | std::ios::badbit);

        process_file(po, json);
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    catch (...) {
        std::cerr << "Unknown error\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
