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
    }
    return outputFile;
}

#ifdef PROFILING
class Profiling
{
private:
    std::chrono::time_point<std::chrono::system_clock> t1, t2;
    std::chrono::duration<double, std::milli> ms_double;
    const char* functionName;

public:
    Profiling(const char* str)
    {
        ms_double = std::chrono::duration<double, std::milli>(-1);
        functionName = str;
        Begin_Profiling();
    }

    void Begin_Profiling()
    {
        t1 = std::chrono::high_resolution_clock::now();
    }

    void End_Profiling()
    {
        t2 = std::chrono::high_resolution_clock::now();

        /* Getting number of milliseconds as a double. */
        ms_double = t2 - t1;

        Show_Profiling_Results();
    }

    void Show_Profiling_Results()
    {
        std::cout << functionName << " : " << ms_double.count() << "ms\n";
    }
};
#endif

int main()
{
#ifdef PROFILING
    Profiling profiling = Profiling(__FUNCTION__);
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
    profiling.End_Profiling();
#endif

    return 0;
}
