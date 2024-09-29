#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>

#ifdef PROFILING
#include <chrono>
#endif

constexpr char INPUT_FILE_NAME[]  = "text4.in";
constexpr char OUTPUT_FILE_NAME[] = "text4.out";

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

int main()
{
    #ifdef PROFILING
    Profiling profiling = Profiling(__PRETTY_FUNCTION__);
    #endif

    IO& io = IO::GetInstance(INPUT_FILE_NAME, OUTPUT_FILE_NAME);

    // Read the entire file into a buffer.
    std::stringstream buffer;
    buffer << io.IN.rdbuf();

    int LINE_SETTING;
    buffer >> LINE_SETTING;
    buffer.ignore(); // Consume the newline character.

    while (buffer.eof() == false)
    {
        std::string line;
        getline(buffer, line);

        std::stringstream ss(line);

        // Clear the string.
        line.clear();

        std::vector<std::string> words;
        std::string              word;

        words.emplace_back("");
        while (ss >> word)
        {
            words.push_back(word);
        }

        // Clear the stringstream.
        ss.clear();
        ss.str(std::string());

        unsigned int current_line_char_length = words[1].size();
        unsigned int current_line_start_index = 1;
        bool         final_word_printed       = false;
        for (unsigned int i = 2; i < words.size(); i++)
        {
            std::string& current_word = words[i];

            if ((i + 1) == words.size())
            {
                if (current_line_char_length + current_word.size() + 1 <= LINE_SETTING)
                {
                    for (unsigned int j = current_line_start_index; j < i; j++)
                    {
                        io.OUT << words[j] << " ";
                    }
                    io.OUT << words[i] << "\n";
                    final_word_printed = true;
                    break;
                }
                final_word_printed = false;
            }

            if (current_line_char_length + current_word.size() + 1 > LINE_SETTING)
            {
                if (i == current_line_start_index + 1)
                {
                    io.OUT << words[current_line_start_index] << "\n";
                    current_line_start_index = i;
                    current_line_char_length = current_word.size();
                    continue;
                }

                unsigned int space_between_words = (LINE_SETTING - current_line_char_length + 1) /
                                                   (i - current_line_start_index);
                unsigned int more_space_instances = (LINE_SETTING - current_line_char_length + 1) %
                                                    (i - current_line_start_index);

                for (unsigned int j = current_line_start_index; j + 1 < i; j++)
                {
                    io.OUT << words[j] << " ";

                    for (unsigned int k = 1; k <= space_between_words; k++)
                    {
                        io.OUT << " ";
                    }

                    if (more_space_instances > 0)
                    {
                        io.OUT << " ";
                        more_space_instances--;
                    }
                }

                io.OUT << words[i - 1] << "\n";

                current_line_start_index = i;
                current_line_char_length = current_word.size() + 1;
            }
            else
            {
                current_line_char_length += current_word.size() + 1;
            }
        }

        if (final_word_printed == false)
        {
            io.OUT << words.back() << "\n";
        }
    }

    #ifdef PROFILING
    profiling.End_Profiling();
    #endif

    return 0;
}
