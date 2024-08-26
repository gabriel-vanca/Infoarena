#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main()
{
    printf("Hello, from Hello_World!\n");

    vector<string> msg{"Hello", "C++", "World", "from", "VS Code", "and the C++ extension!"};

    for (const string& word : msg)
    {
        printf("%s ", word.c_str());
    }
    printf("\n\n");

    for (const string& word : msg)
    {
        cout << word << " ";
    }

    cout << endl << endl;

    return 0;
}
