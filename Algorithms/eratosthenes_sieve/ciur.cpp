#include <bitset>
#include <cassert>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <unordered_map>

#ifdef PROFILING
#include <chrono>
#endif

constexpr char INPUT_FILE_NAME[]  = "ciur.in";
constexpr char OUTPUT_FILE_NAME[] = "ciur.out";

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


/* Notes on performance:
 *   - bitset is the fastest, but size must be known at compile time
 *      which means it is not the most memory efficient
 *   - vector<bool> implements bits optimisation and the size can
 *      be set at run-time which makes it the most memory efficient
 *      by far, but slightly slower than bitset (1-2ms) due to the
 *      extra bit-calculations. O2 and O3 greatly enhance speed.
 *   - bool[] is usually faster than array<bool> but slower than vector<bool>;
 *      as size is determined at run-time, it is more memory efficient than array<bool>
 *      Variable Length Arrays (VLA) are however very buggy and not recommended.
 *   - array<bool> is the least memory efficient as size has to be known
 *      at compile-time and doesn't have the bits optimisations
 *      of vector<bool>; it is also slightly slower
 *      than vector<bool> (1-3ms) and than bool[] (1-2ms), making it both
 *      the slowest and the least memory efficient solution.
 *
 *   As we are more time limited (50ms) than memory-limited (7Mb),
 *      we have chosen bitset.
 *   Our bitset memory performance: 303kb memory.
 */

/* Eratosthenes' sieve with Sundaram optimisation
 *                        & bit optimisation
 *                        & memory optimisation
 *
 * https://web.archive.org/web/20240304075320/https://infoarena.ro/ciurul-lui-eratostene
 * https://en.wikipedia.org/wiki/Sieve_of_Eratosthenes
 * https://en.wikipedia.org/wiki/Sieve_of_Sundaram
 */

unsigned int get_prime_numbers_count(const unsigned int N)
{
    constexpr unsigned int UPPER_RANGE = 2'000'000 / 2 + 1;
    const unsigned int     N_half      = N / 2;
    const unsigned int     N_sqrt      = std::ceil(std::sqrt(N));

    /*
     * sieve[i] == false if 2*i + 1 is prime
     */
    std::bitset<UPPER_RANGE> sieve; // zero-initialised

    for (unsigned int i = 1; i <= N_sqrt; i++)
    {
        if (!sieve[i])
        {
            const unsigned int i_double = i << 1;

            for (unsigned int j = (i + 1) * i_double; j < N_half; j += i_double + 1)
            {
                sieve[j] = true;
            }
        }
    }

    /* bitset.count returns the number of bits that are set to true.
     * 1ms faster than a for loop.
     */
    return N_half - sieve.count();
}

int main()
{
    #ifdef PROFILING
    Profiling profiling = Profiling(__PRETTY_FUNCTION__,
                                    "Eratosthenes' sieve with Sundaram optimisation & bit optimisation & memory optimisation.");
    #endif

    IO& io = IO::GetInstance(INPUT_FILE_NAME, OUTPUT_FILE_NAME);

    unsigned int N; // 2 ≤ N ≤ 2 000 000

    io.IN >> N;
    io.OUT << get_prime_numbers_count(N) << std::endl;

    #ifdef PROFILING
    profiling.End_Profiling();
    #endif

    return 0;
}










