#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>

#ifdef PROFILING
#include <chrono>
#endif

constexpr char INPUT_FILE_NAME[]  = "cautbin.in";
constexpr char OUTPUT_FILE_NAME[] = "cautbin.out";

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

unsigned int log2_32(uint32_t value)
{
    const int tab32[32] = {
        0,
        9,
        1,
        10,
        13,
        21,
        2,
        29,
        11,
        14,
        16,
        18,
        22,
        25,
        3,
        30,
        8,
        12,
        20,
        28,
        15,
        17,
        24,
        7,
        19,
        27,
        23,
        6,
        26,
        5,
        4,
        31
    };

    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    return tab32[(uint32_t)(value * 0x07C4ACDD) >> 27];
}

unsigned int binary_search(const std::vector<int>& numbers,
                           const int               _value,
                           unsigned int            step,
                           const unsigned int      lower_bound,
                           const unsigned int      upper_bound)
{
    unsigned int index = lower_bound;
    while (step)
    {
        if (index + step <= upper_bound && numbers[index + step] <= _value)
        {
            index += step;
        }
        step >>= 1;
    }

    return index;
}

unsigned int reverse_binary_search(const std::vector<int>& numbers,
                                   const int               _value,
                                   unsigned int            step,
                                   const unsigned int      lower_bound,
                                   const unsigned int      upper_bound)
{
    unsigned int index = upper_bound;
    while (step)
    {
        if (index - step >= lower_bound && numbers[index - step] >= _value)
        {
            index -= step;
        }
        step >>= 1;
    }

    return index;
}

int main()
{
    #ifdef PROFILING
    Profiling profiling = Profiling(__PRETTY_FUNCTION__);
    #endif

    IO& io = IO::GetInstance(INPUT_FILE_NAME, OUTPUT_FILE_NAME);

    unsigned int array_size;
    io.IN >> array_size; // 1 ≤ array_size ≤ 100 000

    std::vector<int>   numbers(array_size + 1); // INT_MIN ≤ numbers[i] ≤ INT_MAX; numbers[i] <= numbers[i+1]
    const unsigned int log2_array_size = log2_32(array_size);
    for (unsigned int i = 1; i <= array_size; i++)
    {
        io.IN >> numbers[i];
    }

    unsigned int queries_count; // 1 ≤ queries_count ≤ 100 000
    io.IN >> queries_count;

    while (queries_count--)
    {
        short query_type;  // 0 ≤ query_type ≤ 2
        int   query_value; // INT_MIN ≤ query_value ≤ INT_MAX
        io.IN >> query_type >> query_value;

        switch (query_type)
        {
            case 0:
                {
                    // Find the first occurrence of query_value in the array.
                    // If it exists, return the position of the last occurrence.
                    // If it doesn't exist, return -1.
                    const unsigned int position = binary_search(numbers, query_value, log2_array_size, 1, array_size);
                    if (numbers[position] != query_value)
                    {
                        io.OUT << "-1\n";
                    }
                    else
                    {
                        io.OUT << position << '\n';
                    }
                    break;
                }
            case 1:
                {
                    // Find the last number smaller or equal than query_value.
                    const unsigned int position = binary_search(numbers, query_value, log2_array_size, 1, array_size);
                    io.OUT << position << '\n';
                    break;
                }
            case 2:
                {
                    // Find the first number greater or equal than query_value.
                    const unsigned int position = reverse_binary_search(numbers,
                                                                        query_value,
                                                                        log2_array_size,
                                                                        1,
                                                                        array_size);
                    io.OUT << position << '\n';
                    break;
                }
            default:
                {
                    std::cerr << "ERROR: Invalid query type.\n";
                    io.OUT << "ERROR: Invalid query type.\n";
                    assert(false);
                }
        }
    }

    #ifdef PROFILING
    profiling.End_Profiling();
    #endif

    return 0;
}
