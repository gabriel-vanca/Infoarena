#include <algorithm>
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
        string tempstr;
        copy(word.begin(), word.end(), back_inserter(tempstr));
        // https://stackoverflow.com/questions/54297642/why-use-stdback-inserter-instead-of-end-during-stdcopy
        std::transform(tempstr.begin(), tempstr.end(), tempstr.begin(), ::toupper);
        cout << tempstr << " ";
    }

    cout << endl << endl;

    return 0;
}
