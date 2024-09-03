#include <cassert>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>

constexpr char INPUT_FILE_NAME[]  = "adunare.in";
constexpr char OUTPUT_FILE_NAME[] = "adunare.out";

class IO_LEGACY
{
    public:
        FILE* IN  = nullptr;
        FILE* OUT = nullptr;

        enum StreamType
        {
            READ,
            WRITE
        };

        IO_LEGACY(const char input_file_name[], const char output_file_name[])
        {
            IN  = GetStream(input_file_name, READ);
            OUT = GetStream(output_file_name, WRITE);
        }

        void Close_IN() const
        {
            CloseStream(IN);
        }

        void Close_OUT() const
        {
            CloseStream(OUT);
        }

    private:
        static FILE* GetStream(const char fileName[], const StreamType _streamType)
        {
            const char* _mode = nullptr;
            switch (_streamType)
            {
                case READ:
                    {
                        _mode = "r";
                        break;
                    }
                case WRITE:
                    {
                        _mode = "w";
                        break;
                    }
            }
            FILE* _file = fopen(fileName, _mode);
            if (!_file)
            {
                // https://cplusplus.com/reference/system_error/errc/
                fprintf(stderr,
                        "Failed to open file: %s\nERROR: %s\n       ",
                        fileName,
                        strerror(errno));
                switch (errno)
                {
                    case ENOENT:
                        fprintf(stderr, "File does not exist.\n");
                        break;
                    case EACCES:
                        fprintf(stderr, "Permission denied.\n");
                        break;
                    case EEXIST:
                        fprintf(stderr, "File already exists.\n");
                        break;
                    case EISDIR:
                        fprintf(stderr, "File is a directory.\n");
                        break;
                    case ENOSPC:
                        fprintf(stderr, "No space left on device.\n");
                        break;
                    case EROFS:
                        fprintf(stderr, "Read-only file system.\n");
                        break;
                    case ETXTBSY:
                        fprintf(stderr, "Text file busy.\n");
                        break;
                    default:
                        fprintf(stderr, "Unlisted error type.\n");
                        break;
                }

                assert(_file);
            }

            if (std::ferror(_file))
            {
                fprintf(stderr, "Error handling stream in file: %s\nERROR: %s\n", fileName, strerror(errno));
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

    private:
        void GetInputStream(const char _input_file_name[])
        {
            IN.open(_input_file_name);
            if (!IN.is_open()) // Check if the open operation failed
            {
                std::cerr << "Failed to open input inputFile: " << _input_file_name << std::endl;

                // Check for specific error conditions
                if (IN.bad())
                {
                    std::cerr << "Fatal I/O error: bad-bit is set." << std::endl;
                }

                if (IN.fail())
                {
                    std::cerr << "ERROR: " << strerror(errno) << std::endl;
                }

                assert(IN);
            }
        }

        void GetOutputStream(const char _output_file_name[])
        {
            OUT.open(_output_file_name);
            if (!OUT.is_open())
            {
                std::cerr << "Failed to open output outputFile: " << _output_file_name << std::endl;

                if (OUT.bad())
                {
                    std::cerr << "Fatal I/O error: bad-bit is set." << std::endl;
                }

                if (OUT.fail())
                {
                    std::cerr << "ERROR: " << strerror(errno) << std::endl;
                }

                assert(OUT);
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

    public:
        explicit Profiling(const char* _functionName)
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

        void Show_Profiling_Results() const
        {
            std::cout << functionName << " : "
                    << duration_nano.count() / 1000000 << "ms | "
                    << duration_nano.count() / 1000 << "\xE6s | "
                    << duration_nano.count() << "ns\n";
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
    Profiling profiling = Profiling(__FUNCTION__);
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
