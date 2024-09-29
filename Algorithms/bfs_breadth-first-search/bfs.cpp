#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <queue>
#include <sstream>
#include <unordered_map>
#include <vector>

#ifdef PROFILING
#include <chrono>
#endif

constexpr char INPUT_FILE_NAME[]  = "bfs.in";
constexpr char OUTPUT_FILE_NAME[] = "bfs.out";

class IO_Base;
class IO;
class Profiling;

class Vertex;
class Edge;
class Graph;

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

class Edge
{
    public:
        const Vertex* origin;
        const Vertex* destination;

        Edge(const Vertex* _origin, const Vertex* _destination)
            : origin(_origin), destination(_destination)
        {
        }

        ~Edge() = default;
};

class Vertex
{
    protected:
        std::vector<std::unique_ptr<Edge>> edges;

    public:
        const unsigned int id;
        mutable int        distance = -1;

        explicit Vertex(const unsigned int _id)
            : id(_id)
        {
        }

        ~Vertex() = default;
        // Delete copy constructor. Vertices should not be cloneable.
        Vertex(const Vertex&) = delete;
        // Delete assignment operator. Vertices should not be assignable.
        Vertex& operator=(const Vertex&) = delete;

        void AddVertex(Vertex* _destination)
        {
            edges.push_back(std::make_unique<Edge>(this, _destination));
        }

        long long unsigned int GetNumberOfEdges() const
        {
            return edges.size();
        }

        const Vertex* operator[](const unsigned int _index) const
        {
            return edges[_index]->destination;
        }

        bool operator ==(const Vertex& _vertex) const
        {
            return id == _vertex.id;
        }

        bool operator !=(const Vertex& _vertex) const
        {
            return id != _vertex.id;
        }
};

class Graph
{
    protected:
        std::vector<Vertex*> vertices;

    public:
        explicit Graph(const unsigned int _size)
        {
            vertices.reserve(_size + 1);
            for (unsigned int i = 0; i <= _size; ++i)
            {
                vertices.push_back(new Vertex(i));
            }
        }

        void AddVertex(const unsigned int _origin_id, const unsigned int _destination_id) const
        {
            Vertex* _origin      = vertices[_origin_id];
            Vertex* _destination = vertices[_destination_id];
            _origin->AddVertex(_destination);
        }

        Vertex* operator [](const unsigned int _index) const
        {
            return vertices[_index];
        }

        long long unsigned int GetNumberOfVertices() const
        {
            return vertices.size() - 1;
        }

        bool IsEmpty() const
        {
            return GetNumberOfVertices() == 0;
        }

        ~Graph()
        {
            for (const auto& vertex : vertices)
            {
                delete vertex;
            }
        }

        void BreadthFirstSearch(const unsigned int _source_vertex_id) const
        {
            std::queue<const Vertex*> vertex_queue;
            const Vertex*             source_vertex = vertices[_source_vertex_id];
            source_vertex->distance                 = 0;
            vertex_queue.push(source_vertex);

            while (!vertex_queue.empty())
            {
                const Vertex* current_vertex = vertex_queue.front();
                vertex_queue.pop();

                for (unsigned int i = 0; i < current_vertex->GetNumberOfEdges(); ++i)
                {
                    const Vertex* neighbour_vertex = (*current_vertex)[i];
                    if (neighbour_vertex->distance != -1)
                    {
                        // Vertex is either the source or has been visited.
                        // Nothing to do. Skip to the next neighbour.
                        continue;
                    }

                    // Vertex has not been visited yet.
                    // Set the distance and add it to the queue.
                    neighbour_vertex->distance = current_vertex->distance + 1;
                    vertex_queue.push(neighbour_vertex);
                }
            }
        }

        std::string PrintDistances() const
        {
            std::ostringstream out;
            for (unsigned int i = 1; i <= GetNumberOfVertices(); ++i)
            {
                out << vertices[i]->distance << " ";
            }

            out.seekp(-1);
            out << "\n";

            #ifdef PROFILING
            std::cout << out.str();
            #endif

            return out.str();
        }
};

int main()
{
    #ifdef PROFILING
    Profiling profiling = Profiling(__PRETTY_FUNCTION__);
    #endif

    IO& io = IO::GetInstance(INPUT_FILE_NAME, OUTPUT_FILE_NAME);

    unsigned int NumberOfVertices; // 2 ≤ NumberOfVertices ≤ 100'000
    unsigned int NumberOfEdges;    // 1 ≤ NumberOfEdges ≤ 1'000'000
    unsigned int SourceVertex;

    io.IN >> NumberOfVertices >> NumberOfEdges >> SourceVertex;

    const Graph graph(NumberOfVertices);

    for (unsigned int i = 0; i < NumberOfEdges; ++i)
    {
        unsigned int origin, destination;
        io.IN >> origin >> destination;
        graph.AddVertex(origin, destination);
    }

    graph.BreadthFirstSearch(SourceVertex);
    io.OUT << graph.PrintDistances();

    #ifdef PROFILING
    profiling.End_Profiling();
    #endif

    return 0;
}
