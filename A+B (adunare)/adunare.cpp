#include <cassert>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>

constexpr char INPUT_FILE_NAME[] = "adunare.in";
constexpr char OUTPUT_FILE_NAME[] = "adunare.out";

std::ifstream GetReadFile()
{
    std::ifstream inputFile(INPUT_FILE_NAME);
    if (!inputFile.is_open()) // Check if the open operation failed
    {
        std::cerr << "Failed to open input inputFile: " << INPUT_FILE_NAME << std::endl;

        // Check for specific error conditions
        if (inputFile.bad())
        {
            std::cerr << "Fatal I/O error: badbit is set." << std::endl;
        }

        if (inputFile.fail())
        {
            std::cerr << "Error details: " << strerror(errno) << std::endl;
        }

        return nullptr;
    }
    return inputFile;
}

std::ofstream GetWriteFile()
{
    std::ofstream outputFile(OUTPUT_FILE_NAME);
    if (!outputFile.is_open())
    {
        std::cerr << "Failed to open output outputFile: " << OUTPUT_FILE_NAME << std::endl;

        if (outputFile.bad())
        {
            std::cerr << "Fatal I/O error: badbit is set." << std::endl;
        }

        if (outputFile.fail())
        {
            std::cerr << "Error details: " << strerror(errno) << std::endl;
        }
        return nullptr;
    }
    return outputFile;
}

int main()
{
#ifdef PROFILING
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;
    auto t1 = high_resolution_clock::now();
#endif

    std::ifstream inputFile = GetReadFile();
    assert(inputFile);

    // Read the two numbers from the input inputFile
    int a, b;
    inputFile >> a >> b;

    // Close the input inputFile
    inputFile.close();

    // Compute the sum
    int sum = a + b;

    std::ofstream outputFile = GetWriteFile();
    assert(outputFile);

    outputFile << sum << std::endl;

    // Close the output inputFile
    outputFile.close();

#ifdef PROFILING
    auto t2 = high_resolution_clock::now();

    /* Getting number of milliseconds as an integer. */
    auto ms_int = duration_cast<milliseconds>(t2 - t1);

    /* Getting number of milliseconds as a double. */
    duration<double, std::milli> ms_double = t2 - t1;

    std::cout << ms_int.count() << "ms\n";
    std::cout << ms_double.count() << "ms\n";
#endif

    return 0;
}
