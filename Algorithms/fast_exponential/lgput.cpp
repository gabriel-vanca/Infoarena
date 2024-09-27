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


class IO_LEGACY final : IO_Base
{
    // C I/O functions: https://en.cppreference.com/w/c/io

    protected:
        // The Singleton has a private constructor to prevent direct instantiation.
        IO_LEGACY(const char input_file_name[], const char output_file_name[])
        {
            IN  = GetStream(input_file_name, StreamType::READ);
            OUT = GetStream(output_file_name, StreamType::WRITE);
        }

        // The Singleton has a private destructor to prevent deletion.
        ~IO_LEGACY() override
        {
            is_instance_destroyed() = true;
            IO_LEGACY::Close_IN();
            IO_LEGACY::Close_OUT();
        }

    public:
        FILE* IN  = nullptr;
        FILE* OUT = nullptr;

        // Delete copy constructor. Singletons should not be cloneable.
        IO_LEGACY(const IO_LEGACY&) = delete;
        // Delete move constructor. Singletons should not be movable.
        IO_LEGACY(const IO_LEGACY&&) = delete;
        // Delete assignment operator. Singletons should not be assignable.
        IO_LEGACY& operator=(const IO_LEGACY&) = delete;

        /* Singleton pattern. Only one instance of the class can exist.
         * Thread safe: Initialization is guaranteed to happen only once.
         * A static member object instance is declared. This object is only created
         * the first time the function is called. Static local variables are
         * guaranteed to be initialized only once, even in multithreaded environments.
         * Subsequent calls to GetInstance() simply return the existing instance object.
         * Returning reference instead of pointer further discourages attempts to delete.
         */
        static IO_LEGACY& GetInstance(const char input_file_name[], const char output_file_name[])
        {
            static IO_LEGACY io_Instance(input_file_name, output_file_name);

            if (is_instance_destroyed())
            {
                // We check for The Dead Reference Problem.
                // Our singleton is designed to only be destroyed at program termination.
                fprintf(stderr, "ERROR: Attempt to access destroyed singleton instance.\n");
                assert(false);
            }

            return io_Instance;
        }

    private:
        enum class StreamType
        {
            READ,
            WRITE
        };

        static bool& is_instance_destroyed()
        {
            /* This variable is used to check for The Dead Reference Problem
             * by enabling the class to check if its singleton has been destroyed.
             */
            static bool is_instance_destroyed = false;
            return is_instance_destroyed;
        }

        FILE* GetStream(const char fileName[], const StreamType _streamType)
        {
            const char* _mode = nullptr;
            std::string _error_file_type;
            switch (_streamType)
            {
                case StreamType::READ:
                    {
                        _mode            = "r";
                        _error_file_type = "input";
                        break;
                    }
                case StreamType::WRITE:
                    {
                        _mode            = "w";
                        _error_file_type = "output";
                        break;
                    }
            }
            FILE* _file = fopen(fileName, _mode);
            if (!_file)
            {
                PrintError(fileName, errno, "Failed to open " + _error_file_type);
                assert(_file);
            }

            if (std::ferror(_file))
            {
                PrintError(fileName, errno, ": Error handling stream " + _error_file_type);
                assert(false);
            }

            return _file;
        }

        static void CloseStream(FILE* file)
        {
            fclose(file);
        }

        void Close_IN() override
        {
            CloseStream(IN);
        }

        void Close_OUT() override
        {
            CloseStream(OUT);
        }

        void PrintError(const char* const  _file_name,
                        const int          _error_num,
                        const std::string& _error_source) override
        {
            int error_code = -1;
            if (FILE_OPEN_ERROR.find(_error_num) != FILE_OPEN_ERROR.end())
            {
                error_code = _error_num;
            }
            fprintf(stderr,
                    "%s file: %s\nERROR: %s\n       %s\n",
                    _error_source.c_str(),
                    _file_name,
                    strerror(errno),
                    FILE_OPEN_ERROR.at(error_code).c_str());
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

int main()
{
    #ifdef PROFILING
    Profiling profiling = Profiling(__PRETTY_FUNCTION__);
    #endif

    IO_LEGACY& io = IO_LEGACY::GetInstance(INPUT_FILE_NAME, OUTPUT_FILE_NAME);

    int a, b;
    fscanf(io.IN, "%d %d", &a, &b);
    fprintf(io.OUT, "%d\n", a + b);

    #ifdef PROFILING
    profiling.End_Profiling();
    #endif

    return 0;
}
