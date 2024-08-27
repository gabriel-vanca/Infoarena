#include <iostream>
#include <fstream>

int main()
{
    // Open the input file
    std::ifstream inputFile("adunare.in");
    if (!inputFile || inputFile.fail() || inputFile.bad())
    {
        std::cerr << "Failed to open input file." << std::endl;
        return 1;
    }

    // Read the two numbers from the input file
    int a, b;
    inputFile >> a >> b;

    // Close the input file
    inputFile.close();

    // Compute the sum
    int sum = a + b;

    // Open the output file
    std::ofstream outputFile("adunare.out");
    if (!outputFile || outputFile.fail() || outputFile.bad() || !outputFile.is_open())
    {
        std::cerr << "Failed to open output file." << std::endl;
        return 1;
    }

    // Write the sum to the output file

    outputFile << sum << std::endl;

    // Close the output file
    outputFile.close();

    return 0;
}
