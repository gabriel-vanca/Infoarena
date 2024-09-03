#include <cassert>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <unordered_map>

constexpr char INPUT_FILE_NAME[]  = "adunare.in";
constexpr char OUTPUT_FILE_NAME[] = "adunare.out";

class IO_LEGACY
{
    public:
        FILE* IN  = nullptr;
        FILE* OUT = nullptr;

        enum class StreamType
        {
            READ,
            WRITE
        };

        IO_LEGACY(const char input_file_name[], const char output_file_name[])
        {
            IN  = GetStream(input_file_name, StreamType::READ);
            OUT = GetStream(output_file_name, StreamType::WRITE);
        }

        void Close_IN() const
        {
            CloseStream(IN);
        }

        void Close_OUT() const
        {
            CloseStream(OUT);
        }

        // https://cplusplus.com/reference/system_error/errc/
        std::unordered_map<int, const std::string> const FILE_OPEN_ERROR = {
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

        void PrintError(const char* const _file_name, const int _error_num, const std::string& _error_source) const
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

    private:
        FILE* GetStream(const char fileName[], const StreamType _streamType) const
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
};


class IO
{
    public:
        std::ifstream IN;
        std::ofstream OUT;

        IO(const char input_file_name[], const char output_file_name[])
        {
            GetInputStream(input_file_name);
            GetOutputStream(output_file_name);
        }

        void Close_IN()
        {
            IN.close();
        }

        void Close_OUT()
        {
            OUT.close();
        }

        // https://cplusplus.com/reference/system_error/errc/
        std::unordered_map<int, const std::string> const FILE_OPEN_ERROR = {
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

        void PrintError(const char* const _file_name, const int _error_num, const std::string& _error_source) const
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

    private:
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
        explicit Profiling(const char* _functionName, const char* _comment)
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
                    << duration_nano.count() / 1000000 << "ms | "
                    << duration_nano.count() / 1000 << "\xE6s | "
                    << duration_nano.count() << "ns\n"
                    << "       " << comment << "\n";
        }
};
#endif

void Add()
{
    IO io = IO(INPUT_FILE_NAME, OUTPUT_FILE_NAME);

    int a, b;
    io.IN >> a >> b;
    io.Close_IN();

    io.OUT << a + b << std::endl;
    io.Close_OUT();
}

void Add_Legacy()
{
    const IO_LEGACY io = IO_LEGACY(INPUT_FILE_NAME, OUTPUT_FILE_NAME);

    int a, b;
    fscanf(io.IN, "%d %d", &a, &b);
    io.Close_IN();

    fprintf(io.OUT, "%d\n", a + b);
    io.Close_OUT();
}

int main()
{
    #ifdef PROFILING
    Profiling profiling = Profiling(__FUNCTION__, "Add two numbers from a file.");
    #endif

    if (std::rand() % 2)
        Add();
    else
        Add_Legacy();

    #ifdef PROFILING
    profiling.End_Profiling();
    #endif

    return 0;
}
