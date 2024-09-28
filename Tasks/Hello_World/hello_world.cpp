#include <algorithm>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main()
{
    vector<string> msg{"Hello", "World", "from C++!"};

    printf("\n");

    for (const string& word : msg)
    {
        string tempstr;
        copy(word.begin(), word.end(), back_inserter(tempstr));
        // https://stackoverflow.com/questions/54297642/why-use-stdback-inserter-instead-of-end-during-stdcopy
        std::transform(tempstr.begin(), tempstr.end(), tempstr.begin(), ::toupper);
        printf("%s ", tempstr.c_str());
    }
    printf("\n\n");

    return 0;
}
