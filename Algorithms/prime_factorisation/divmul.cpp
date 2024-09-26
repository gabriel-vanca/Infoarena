#include <bitset>
#include <cmath>
#include <vector>
#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <unordered_map>

#ifdef PROFILING
#include <chrono>
#endif

constexpr char INPUT_FILE_NAME[]  = "divmul.in";
constexpr char OUTPUT_FILE_NAME[] = "divmul.out";

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

std::vector<unsigned int> get_prime_numbers(const unsigned int N = 10'100)
{
    constexpr unsigned int UPPER_RANGE = 10'100 / 2 + 1;
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
    const unsigned int        prime_count = N_half - sieve.count();
    std::vector<unsigned int> primes(prime_count);
    primes[0] = 2;
    for (unsigned int i = 1, j = 1; j < prime_count; i++)
    {
        if (!sieve[i])
        {
            primes[j++] = 2 * i + 1;
        }
    }

    return primes;
}

// We pass the vector primes by reference to avoid copying.
// std::map<unsigned int, unsigned int> factorise(unsigned int N, const std::vector<unsigned int>& primes)
unsigned int factorise(unsigned int N, const std::vector<unsigned int>& primes)
{
    //std::map<unsigned int, unsigned int> factors;
    unsigned int factors_counter = 1 - (N & 1);

    while ((N & 1) == 0)
    {
        N >>= 1;
    }

    for (const auto& prime : primes)
    {
        if (N == 1)
        {
            break;
        }

        if (prime * prime > N)
        {
            break;
        }

        if (N % prime == 0)
        {
            factors_counter++;

            do
            {
                // factors[prime]++;
                N /= prime;
            }
            while (N % prime == 0);
        }
    }

    if (N > 1)
    {
        factors_counter++;
    }

    return factors_counter;
}

int main()
{
    #ifdef PROFILING
    Profiling profiling = Profiling(__PRETTY_FUNCTION__, "Fast prime factorisation.");
    #endif

    IO& io = IO::GetInstance(INPUT_FILE_NAME, OUTPUT_FILE_NAME);

    /* Get the prime numbers up to 10'000.
     * We use initialisation rather than assignment
     * so as to trigger named return value optimization (NRVO)
     * and therefore avoid copying.
     */
    const auto primes = get_prime_numbers();

    unsigned int T_counter; // 1 ≤ T ≤ 30'000
    io.IN >> T_counter;

    while (T_counter--)
    {
        unsigned int gcd, lcm;
        io.IN >> gcd; // 2 ≤ gcd ≤ 10.000
        io.IN >> lcm; // 2 ≤ lcm ≤ 100.000.000

        if (lcm == gcd)
        {
            io.OUT << "1\n";
            continue;
        }

        if (gcd > lcm)
        {
            std::swap(gcd, lcm);
        }

        if (lcm == 0 || gcd == 0 || gcd == 1 || lcm == 1)
        {
            io.OUT << "0\n";
            continue;
        }

        if (lcm % gcd != 0)
        {
            io.OUT << "0\n";
            continue;
        }

        const unsigned int differential    = lcm / gcd;
        const unsigned int factors_counter = factorise(differential, primes);

        /* We apply the combinatorics formula to determine the
         * number of k-combinations for all k from 0 to n.
         * ∑[k=0->n] C(n k) = 2^n
         * \textstyle \sum _{0\leq {k}\leq {n}}{\binom {n}{k}}=2^{n}}
         * https://gabriel-vanca.github.io/mathjax-viewer/?input=%7B%5Ctextstyle+%5Csum+_%7B0%5Cleq+%7Bk%7D%5Cleq+%7Bn%7D%7D%7B%5Cbinom+%7Bn%7D%7Bk%7D%7D%3D2%5E%7Bn%7D%7D
         */
        const unsigned long long int solution = 1ULL << factors_counter;
        io.OUT << solution << "\n";
    }

    #ifdef PROFILING
    profiling.End_Profiling();
    #endif

    return 0;
}




