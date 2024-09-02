#include <cassert>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>

constexpr char INPUT_FILE_NAME[] = "adunare.in";
constexpr char OUTPUT_FILE_NAME[] = "adunare.out";

std::ifstream GetInputStream()
{
    std::ifstream inputFile(INPUT_FILE_NAME);
    if (!inputFile.is_open()) // Check if the open operation failed
    {
        std::cerr << "Failed to open input inputFile: " << INPUT_FILE_NAME << std::endl;

        // Check for specific error conditions
        if (inputFile.bad())
        {
            std::cerr << "Fatal I/O error: bad-bit is set." << std::endl;
        }

        if (inputFile.fail())
        {
            std::cerr << "Error details: " << strerror(errno) << std::endl;
        }

        assert(inputFile);
    }
    return inputFile;
}

std::ofstream GetOutputStream()
{
    std::ofstream outputFile(OUTPUT_FILE_NAME);
    if (!outputFile.is_open())
    {
        std::cerr << "Failed to open output outputFile: " << OUTPUT_FILE_NAME << std::endl;

        if (outputFile.bad())
        {
            std::cerr << "Fatal I/O error: bad-bit is set." << std::endl;
        }

        if (outputFile.fail())
        {
            std::cerr << "Error details: " << strerror(errno) << std::endl;
        }

        assert(outputFile);
    }
    return outputFile;
}

#ifdef PROFILING
class Profiling
{
private:
    std::chrono::time_point<std::chrono::system_clock> time_begin, time_end;
    std::chrono::duration<double, std::nano> duration_nano;
    const char* functionName;

public:
    Profiling(const char* _functionName)
    {
        this->functionName = _functionName;
        Begin_Profiling();
    }

    void Begin_Profiling()
    {
        time_begin = std::chrono::high_resolution_clock::now();
    }

    void End_Profiling()
    {
        time_end = std::chrono::high_resolution_clock::now();

        /* Getting number of nanoseconds as a double. */
        duration_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(time_end - time_begin);

        Show_Profiling_Results();
    }

    void Show_Profiling_Results()
    {
        std::cout << functionName << " : "
            << duration_nano.count() / 1000000 << "ms | "
            << duration_nano.count() / 1000 << "\xE6s | "
            << duration_nano.count() << "ns\n";
    }
};
#endif

int main()
{
#ifdef PROFILING
    Profiling profiling = Profiling(__FUNCTION__);
#endif

    std::ifstream inputFile = GetInputStream();

    // Read the two numbers from the input inputFile
    int a, b;
    inputFile >> a >> b;

    // Close the input inputFile
    inputFile.close();

    // Compute the sum
    int sum = a + b;

    std::ofstream outputFile = GetOutputStream();

    outputFile << sum << std::endl;

    // Close the output inputFile
    outputFile.close();

#ifdef PROFILING
    profiling.End_Profiling();
#endif

    return 0;
}
