#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stack>
#include <unordered_map>
#include <vector>

#ifdef PROFILING
#include <chrono>
#endif

constexpr char INPUT_FILE_NAME[]  = "text3.in";
constexpr char OUTPUT_FILE_NAME[] = "text3.out";

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

class Node
{
    public:
        std::string&       word;
        Node*              parent;
        const unsigned int depth;

        /* std::map<char, Node*> children: holds the children of the node.
         * The key is the last letter of the word.
         * The value is the child node.
         * Whilst map is slower at adding objects than unordered_map,
         * the map has 26 elements at most, so the difference is negligible.
         * Also map is faster for seek/find.
         */
        std::map<char, Node*> children;

        explicit Node(std::string& _word, Node* _parent = nullptr)
            : word(_word), parent(_parent), depth(_parent ? (_parent->depth + 1) : 1)
        {
        }

        ~Node() = default;

        bool Is_Root() const
        {
            return parent == nullptr;
        }

        bool Is_Leaf() const
        {
            return children.empty();
        }

        // bool operator==(const Node& _rhs) const
        // {
        //     return word == _rhs.word;
        // }
        //
        // bool operator!=(const Node& _rhs) const
        // {
        //     return word != _rhs.word;
        // }

        bool operator<(const Node& _rhs) const
        {
            return depth < _rhs.depth;
        }

        bool operator>(const Node& _rhs) const
        {
            return depth > _rhs.depth;
        }

        bool operator<=(const Node& _rhs) const
        {
            return depth <= _rhs.depth;
        }

        bool operator>=(const Node& _rhs) const
        {
            return depth >= _rhs.depth;
        }
};

int main()
{
    #ifdef PROFILING
    Profiling profiling = Profiling(__PRETTY_FUNCTION__);
    #endif

    IO& io = IO::GetInstance(INPUT_FILE_NAME, OUTPUT_FILE_NAME);

    // Dump the entire file into a buffer.
    std::stringstream buffer;
    buffer << io.IN.rdbuf();

    std::vector<std::string> words; // max vector size is 20'000; max word size is 20

    // Read the buffer word by word.
    // We don't need to worry about case sensitivity.
    while (buffer.eof() == false)
    {
        std::string line;
        buffer >> line;
        words.push_back(line);
    }

    // We don't need the buffer any more.
    // First we clear the state (set end-of-file flag to false).
    buffer.clear();
    // Then we clear the buffer.
    buffer.str(std::string());

    /* nodes_map <char, Node*>: holds the node with the highest depth
     * It is worth noting that each ending letter could be a root node.
     * Therefore, we have anywhere between one and 26 trees.
     */
    std::map<char, Node*> nodes_map;

    for (std::string& word : words)
    {
        if (isalpha(word.front()) == false || isalpha(word.back()) == false)
        {
            // TODO: can be removed in the final version
            std::cerr << "Invalid word: " << word << std::endl;
            continue;
        }

        Node* new_child_node = nullptr;

        /* First we need to deal with the start of the word.
         * Can we plug this word into another node?
         * If we can't, we need to add it to the root nodes.
         */
        if (nodes_map.find(word.front()) == nodes_map.end())
        {
            /* There is nowhere to plug this node in.
             * We need to check if another left node doesn't already have
             * the same termination. If it does, we don't need this new word.
             * Otherwise, we will add it to the root nodes.
             */
        }
        else
        {
            /* We have a node ending with the same starting letter as our new word.
             * We need to check if it already has a child with the same termination.
             */
            if (nodes_map[word.front()]->children.find(word.back()) != nodes_map[word.front()]->children.end())
            {
                /* We already have a node with the same termination.
                 * An earlier word is preferable than a later one.
                 * We ignore this word.
                 */
                word.clear();
                continue;
            }

            /* We don't have a node with the same termination.
             * We prepare to add this word to the tree, pending no depth conflict.
             */
            new_child_node                                 = new Node(word, nodes_map[word.front()]);
            nodes_map[word.front()]->children[word.back()] = new_child_node;
        }

        /* We now know we are dealing with a node that is either pluggable
         * (if new_child_node is not nullptr) or a root node.
         * Now we need to deal with the end of the word.
         */
        if (nodes_map.find(word.back()) == nodes_map.end())
        {
            /* We don't have a node with the same termination,
             * so we don't need to worry about competing depths.
             */
            if (new_child_node)
            {
                // We have a new max depth for character word.back().
                nodes_map[word.back()] = new_child_node;
            }
            else
            {
                // New root node.
                // We have a new max depth (of 1) for character word.back().
                nodes_map[word.back()] = new Node(word, nullptr);
            }
        }
        else
        {
            /* We already have a node with the same termination.
             * We need to check for competing depths.
             */

            if (new_child_node == nullptr)
            {
                /* This is a root node, therefore its depth is 1.
                 * No need to add it.
                 */
                continue;
            }

            if (nodes_map[word.back()]->depth < new_child_node->depth)
            {
                /* The new word provides us with a higher depth.
                 * We need to and add the new one to the map
                 * We do not delete the old one as it might still be parts
                 * of another sequence.
                 */
                nodes_map[word.back()] = new_child_node;
            }
            else
            {
                /* Out new word has a lower depth.
                 * We don't need to add it.
                 * Not only that, but we need to remove it from the map.
                 */

                nodes_map[word.front()]->children.erase(word.back());
                delete new_child_node;
            }
        }
    }

    for (auto it = nodes_map.cbegin(); it != nodes_map.cend();)
    {
        /* Every node that is not a leaf by definition cannot be
         * having the highest depth. We can remove it now.
         */
        if (it->second == nullptr || it->second->Is_Leaf() == false)
        {
            it = nodes_map.erase(it);
        }
        else
        {
            ++it;
        }
    }

    unsigned int best_depth = 0;
    const Node*  best_leaf  = nullptr;
    for (const auto& [key, node] : nodes_map)
    {
        if (node->depth >= best_depth)
        {
            best_depth = node->depth;
            best_leaf  = node;
        }
    }

    io.OUT << words.size() << "\n";
    io.OUT << words.size() - best_depth << "\n";
    const Node*             current_node = best_leaf;
    std::stack<std::string> word_stack;
    do
    {
        word_stack.push(current_node->word);
        current_node = current_node->parent;
    }
    while (current_node);

    while (word_stack.empty() == false)
    {
        io.OUT << word_stack.top() << "\n";
        word_stack.pop();
    }

    #ifdef PROFILING
    profiling.End_Profiling();
    #endif

    return 0;
}
