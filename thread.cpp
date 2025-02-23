// Directive Space : 

#include <iostream>
#include <iomanip>    // For setw and output of addresses in hex
#include <thread>     // For sleep_for
#include <chrono>     // For the delay time
#include <cstdlib>    // For rand() and system()
#include <ctime>      // To initialize the random number generator


// ----             ----

using namespace std;

// ----             ----



//  --              -------------------------           --
// |     ANSI escape sequences for colorful typography    |
//  --              -------------------------           --

const string RED = "\033[1;31m";
const string GREEN = "\033[1;32m";
const string YELLOW = "\033[1;33m";
const string BLUE = "\033[1;34m";
const string MAGENTA = "\033[1;35m";
const string CYAN = "\033[1;36m";
const string RESET = "\033[0m";


//  --              -------------------------           --

// Main function of the program
int main() {
    // Initializing the random number generator
    srand(static_cast<unsigned>(time(0)));

    // Create an array of 10 integers (real variables)
    int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = rand() % 100;  // random number from 0 to 99
    }

    // An array of pointers to each element of the array
    int* ptrs[10];
    for (int i = 0; i < 10; i++) {
        ptrs[i] = &arr[i];
    }

    // Infinite loop to update data every 4 seconds
    while (true) {
        // Clear console (CLS for Windows, clear for Unix-like systems)
#ifdef _WIN32
        system("CLS");
#else
        system("clear");
#endif

        cout << BLUE << "=== Demonstration of pointers in C++ ===" << RESET << "\n\n";

        // Printing the state of an array and its pointers
        cout << GREEN << "State of array and pointers:" << RESET << "\n";
        for (int i = 0; i < 10; i++) {
            cout << YELLOW << "Element " << setw(2) << i << ":" << RESET
                << " value = " << setw(3) << arr[i]
                << " | address = " << ptrs[i]
                    << " (hex: " << hex << reinterpret_cast<uintptr_t>(ptrs[i]) << dec << ")" << "\n";
        }

        // Demonstration of pointer to pointer
        cout << "\n" << MAGENTA << "Demonstration of pointer to pointer:" << RESET << "\n";
        int* pFirst = &arr[0];    // pointer to the first element
        int** ppFirst = &pFirst;  // pointer to pointer
        cout << "The value of the first element via a pointer: " << *pFirst << "\n";
        cout << "The value of the first element via a pointer to a pointer: " << **ppFirst << "\n";

        // Example of exchanging values ​​via pointers
        cout << "\n" << CYAN << "Exchanging the values ​​of the first and last element via pointers:" << RESET << "\n";
        cout << "Before the exchange : arr[0] = " << arr[0] << ", arr[9] = " << arr[9] << "\n";
        // Exchanging values ​​via pointers
        int temp = *ptrs[0];
        *ptrs[0] = *ptrs[9];
        *ptrs[9] = temp;
        cout << "After the exchange: arr[0] = " << arr[0] << ", arr[9] = " << arr[9] << "\n";

        //Update array values ​​with random numbers for the next iteration
        for (int i = 0; i < 10; i++) {
            arr[i] = rand() % 100;
        }

        cout << "\n" << BLUE << "Next update in 4 seconds..." << RESET << "\n";
        std::this_thread::sleep_for(std::chrono::seconds(4));  
    }

    return 0;
}
