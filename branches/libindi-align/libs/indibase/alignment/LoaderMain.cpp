#include <iostream>

#include "LoaderClient.h"

using namespace std;

static LoaderClient Client;

int main()
{
    Client.Initialise();

    cout << "Press any key to terminate the client.\n";
    string term;
    cin >> term;
    return 0;
}
