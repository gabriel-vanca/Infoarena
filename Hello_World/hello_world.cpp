#include <cstdio>
#include <iostream>
#include <vector>
#include <string>

using namespace std;

int main()
{
    printf("Hello, from Hello_World!\n");

    vector<string> msg{"Hello", "C++", "World", "from", "VS Code", "and the C++ extension!"};

    for (const string &word : msg)
    {
        printf("%s ", word.c_str());
        //  std::print(" ",word);
        cout << word << " ";
    }
    cout << endl
         << endl;

    return 0;
}
