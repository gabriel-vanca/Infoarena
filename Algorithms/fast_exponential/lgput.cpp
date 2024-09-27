#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <unordered_map>

#ifdef PROFILING
#include <chrono>
#endif

constexpr char INPUT_FILE_NAME[]  = "lgput.in";
constexpr char OUTPUT_FILE_NAME[] = "lgput.out";

class IO_Base
{
    protected:
        IO_Base()          = default;
        virtual ~IO_Base() = default;

        // https://cplusplus.com/reference/system_error/errc/
        const std::unordered_map<int, std::string> FILE_OPEN_ERROR = {
            {ENOENT, "File does not exist."},
            {EACCES, "Permission denied."},
            {EEXIST, "File already exists."},
            {EISDIR, "File is a directory."},
            {ENOSPC, "No space left on device."},
            {EROFS, "Read-only file system."},
            {ETXTBSY, "Text file busy."},
            {-1, "Unlisted error type."},
            {0, "No error."}
        };

        virtual void Close_IN() = 0;
        virtual void Close_OUT() = 0;

        virtual void PrintError(const char* const  _file_name,
                                const int          _error_num,
                                const std::string& _error_source) = 0;
};

class IO final : IO_Base
{
    // C++ I/O functions: https://en.cppreference.com/w/cpp/io

    protected:
        // The Singleton has a private constructor to prevent direct instantiation.
        IO(const char input_file_name[], const char output_file_name[])
        {
            GetInputStream(input_file_name);
            GetOutputStream(output_file_name);
        }

        // The Singleton has a private destructor to prevent deletion.
        ~IO() override
        {
            is_instance_destroyed() = true;
            Close_IN();
            Close_OUT();
        }

    public:
        // Don't make these nullptr. They are not pointers.
        std::ifstream IN;
        std::ofstream OUT;

        // Delete copy constructor. Singletons should not be cloneable.
        IO(const IO&) = delete;
        // Delete move constructor. Singletons should not be movable.
        IO(const IO&&) = delete;
        // Delete assignment operator. Singletons should not be assignable.
        IO& operator=(const IO&) = delete;

        /* Singleton pattern. Only one instance of the class can exist.
         * Thread safe: Initialization is guaranteed to happen only once.
         * A static member object instance is declared. This object is only created
         * the first time the function is called. Static local variables are
         * guaranteed to be initialized only once, even in multithreaded environments.
         * Subsequent calls to GetInstance() simply return the existing instance object.
         * Returning reference instead of pointer further discourages attempts to delete.
         */
        static IO& GetInstance(const char input_file_name[], const char output_file_name[])
        {
            static IO io_Instance(input_file_name, output_file_name);

            if (is_instance_destroyed())
            {
                // We check for The Dead Reference Problem.
                // Our singleton is designed to only be destroyed at program termination.
                std::cerr << "ERROR: Attempt to access destroyed singleton instance." << std::endl;
                assert(false);
            }

            return io_Instance;
        }

    private:
        static bool& is_instance_destroyed()
        {
            /* This variable is used to check for The Dead Reference Problem
             * by enabling the class to check if its singleton has been destroyed.
             */
            static bool is_instance_destroyed = false;
            return is_instance_destroyed;
        }

        void GetInputStream(const char _input_file_name[])
        {
            IN.open(_input_file_name);
            if (!IN.is_open()) // Check if the open operation failed
            {
                if (IN.fail())
                {
                    PrintError(_input_file_name, errno, "Failed to open input");
                    assert(IN);
                }

                if (IN.bad())
                {
                    PrintError(_input_file_name, errno, "Fatal I/O error: bad-bit is set in input");
                    assert(IN);
                }
            }
        }

        void GetOutputStream(const char _output_file_name[])
        {
            OUT.open(_output_file_name);
            if (!OUT.is_open()) // Check if the open operation failed
            {
                if (OUT.fail())
                {
                    PrintError(_output_file_name, errno, "Failed to open output");
                    assert(OUT);
                }

                if (OUT.bad())
                {
                    PrintError(_output_file_name, errno, "Fatal I/O error: bad-bit is set in output");
                    assert(OUT);
                }
            }
        }

        void Close_IN() override final
        {
            IN.close();
        }

        void Close_OUT() override final
        {
            OUT.close();
        }

        void PrintError(const char* const  _file_name,
                        const int          _error_num,
                        const std::string& _error_source) final override
        {
            int error_code = -1;
            if (FILE_OPEN_ERROR.find(_error_num) != FILE_OPEN_ERROR.end())
            {
                error_code = _error_num;
            }

            std::cerr << _error_source << " file: " << _file_name << "\n"
                    << "ERROR: " << strerror(errno) << "\n"
                    << "       " << FILE_OPEN_ERROR.at(error_code) << std::endl;
        }
};

#ifdef PROFILING
class Profiling
{
    private:
        std::chrono::time_point<std::chrono::system_clock> time_begin, time_end;
        std::chrono::duration<double, std::nano>           duration_nano = std::chrono::nanoseconds(0);
        const char*                                        functionName;
        const char*                                        comment;

    public:
        explicit Profiling(const char* _functionName, const char* _comment = "")
            : functionName(_functionName), comment(_comment)
        {
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

        void Show_Profiling_Results() const
        {
            std::cout << functionName << " : "
                    << duration_nano.count() / 1'000'000'000 << "s | "
                    << duration_nano.count() / 1'000'000 << "ms | "
                    << duration_nano.count() / 1'000 << "\xE6s | "
                    << duration_nano.count() << "ns\n"
                    << "             " << comment << "\n";
        }
};
#endif

/* Exponentiation by squaring. Fast exponential in logarithmic time. O(log n)
 * Exponentiation by squaring is a general method for fast computation of large positive integer powers of a number.
 * It is based on the observation that, if the exponent n is even, then x^n = (x^2)^(n/2).
 * If n is odd, then x^n = x * x^(n-1).
 *      {\displaystyle x^{n}={\begin{cases}x\,(x^{2})^{(n-1)/2},&{\mbox{if }}n{\mbox{ is odd}}\\(x^{2})^{n/2},&{\mbox{if }}n{\mbox{ is even}}\end{cases}}}
 *      https://gabriel-vanca.github.io/mathjax-viewer/?input=%7B%5Cdisplaystyle+x%5E%7Bn%7D%3D%7B%5Cbegin%7Bcases%7Dx%5C%2C%28x%5E%7B2%7D%29%5E%7B%28n-1%29%2F2%7D%2C%26%7B%5Cmbox%7Bif+%7D%7Dn%7B%5Cmbox%7B+is+odd%7D%7D%5C%5C%28x%5E%7B2%7D%29%5E%7Bn%2F2%7D%2C%26%7B%5Cmbox%7Bif+%7D%7Dn%7B%5Cmbox%7B+is+even%7D%7D%5Cend%7Bcases%7D%7D%7D
 * This allows to divide the exponentiation process into two recursive steps.
 * The algorithm is as follows:
 * 1. If the exponent is 0, return 1.
 * 2. If the exponent is even, return the square of the result of
 *      recursively raising the base to the power of half the exponent.
 * 3. If the exponent is odd, return the base times the square of the result of
 *      recursively raising the base to the power of half the exponent.
 * 4. If the exponent is negative then we can reuse the previous formula by
 *      rewriting the value using a positive exponent:
 *      {\displaystyle x^{n}=\left({\frac {1}{x}}\right)^{-n}\,.}
 *      https://gabriel-vanca.github.io/mathjax-viewer/?input=%7B%5Cdisplaystyle+x%5E%7Bn%7D%3D%5Cleft%28%7B%5Cfrac+%7B1%7D%7Bx%7D%7D%5Cright%29%5E%7B-n%7D%5C%2C.%7D
 * The algorithm is implemented iteratively to avoid the overhead of recursive function calls.
 * The algorithm has a time complexity of O(log n) and a space complexity of O(log n).
 * The algorithm can be used to calculate:
 *      - the modular exponentiation of a number
 *             The modular exponentiation of a number is the remainder of the number raised
 *             to the power of the exponent divided by a modulus.
 *      - combinatorics
 *      - Fibonacci numbers
 *      - matrix exponentiation
 *      - number of paths of length k in a graph
 * https://cp-algorithms.com/algebra/binary-exp.html
 * https://en.wikipedia.org/wiki/Exponentiation_by_squaring
 */

unsigned long long FastExponential(unsigned long long int       base,
                                   unsigned long long int       exponent,
                                   const unsigned long long int mod)
{
    unsigned long long int result = 1;

    while (exponent > 0)
    {
        if (exponent & 1) // If the exponent is odd
        {
            result = result * base % mod;
            exponent--;
        }

        base = base * base % mod;
        exponent >>= 1; // Divide the exponent by 2
    }

    return result;
}

int main()
{
    #ifdef PROFILING
    Profiling profiling = Profiling(__PRETTY_FUNCTION__,
                                    "Exponentiation by squaring. Fast exponential in logarithmic time. O(log n)");
    #endif

    IO& io = IO::GetInstance(INPUT_FILE_NAME, OUTPUT_FILE_NAME);

    constexpr unsigned long long int MOD = 1'999'999'973;
    unsigned long long int           N; // Base.  2 ≤ N ≤ 2^32
    unsigned long long int           P; // Exponent. 2 ≤ P ≤ 2^32

    io.IN >> N >> P;
    io.OUT << FastExponential(N, P, MOD) << std::endl;

    #ifdef PROFILING
    profiling.End_Profiling();
    #endif

    return 0;
}
