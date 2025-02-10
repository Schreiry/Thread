# Matrix Multiplication with Threads

This project is a multithreaded C++ application that performs matrix multiplication concurrently across multiple threads while dynamically scaling the matrix size after each complete multiplication cycle. It serves as a creative playground for exploring C++ concurrency, synchronization, and dynamic resource management.


## Overview

In this project, **four threads** perform matrix multiplication on shared global matrices, and a **fifth thread** generates new matrices by doubling their size after every cycle. Although the approach is not optimized for efficiency or industrial standards, it reflects the passion of an enthusiast who is exploring C++ in a spontaneous and nonconventional manner.


## Key Components

### Global Variables and Initialization
- **Matrix Variables:**  
  `globalA` and `globalB` hold the matrices used for multiplication. Their initial size is determined by `currentN` (starting at 10).
- **Synchronization Tools:**  
  A `std::mutex` (`cv_mtx`) and a `std::condition_variable` (`cv`) are used to coordinate between the multiplication threads and the generator thread. The integer `finishedCount` keeps track of how many multiplication threads have completed their work.

### Matrix Multiplication Function
- **`multiplyMatrices`:**  
  This function performs standard matrix multiplication. For each element in the resulting matrix `C`, it computes the dot product of the corresponding row from matrix `A` and column from matrix `B`.

### Multiplication Threads
- **`multiplicationThread`:**  
  Each of the four threads repeatedly performs the following steps:
  - Reads the current matrix size.
  - Allocates a local result matrix `C`.
  - Measures the time taken to multiply the global matrices.
  - Outputs a well-formatted message showing the thread number, matrix size, and elapsed time.
  - Increases a synchronization counter (`finishedCount`) and waits for the generator thread to signal that new matrices are ready before starting the next cycle.

### Matrix Generator Thread
- **`matrixGeneratorThread`:**  
  This thread waits until all four multiplication threads finish their current cycle (i.e., when `finishedCount` equals 4). Then, it:
  - Doubles the matrix size (`currentN`).
  - Generates new matrices: `globalA` is filled with ones and `globalB` with twos.
  - Resets the synchronization counter.
  - Notifies all waiting multiplication threads to begin processing with the new matrices.

### Main Function
- **`main`:**  
  Initializes the global matrices with the starting size, creates the four multiplication threads and the matrix generator thread, and then joins all threads. (Note: the program is designed to run indefinitely.)

---

## Code Listing

```cpp
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>
#include <vector>
#include <condition_variable>

using namespace std;

// Global variables for matrices and their size
int currentN = 10; // Initial matrix size
vector<vector<int>> globalA;
vector<vector<int>> globalB;

// Mutex and condition variable for synchronization
mutex cv_mtx;
condition_variable cv;
int finishedCount = 0; // Count of completed multiplication threads

// Standard matrix multiplication function
void multiplyMatrices(const vector<vector<int>>& A, const vector<vector<int>>& B,
    vector<vector<int>>& C, int N) {
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            int sum = 0;
            for (int k = 0; k < N; ++k) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}

// Function executed by multiplication threads
void multiplicationThread(int threadId) {
    while (true) {
        // Read the current matrix size
        int N_local = currentN;

        // Local result matrix
        vector<vector<int>> C(N_local, vector<int>(N_local, 0));

        // Measure the start time of multiplication
        auto start = chrono::high_resolution_clock::now();

        // Perform multiplication on global matrices (assuming updates happen only between cycles)
        multiplyMatrices(globalA, globalB, C, N_local);

        // Measure end time and calculate duration
        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double> duration = end - start;

        // Formatted console output with decorative separators
        cout << "=============================================" << endl;
        cout << ">>> Thread " << threadId << " completed matrix multiplication" << endl;
        cout << "Matrix size : " << N_local << "x" << N_local << endl;
        cout << "Lead time   : [" << duration.count() << " seconds]" << endl;
        cout << "=============================================" << endl;

        // Synchronization: signal that this thread finished multiplication
        {
            unique_lock<mutex> lk(cv_mtx);
            finishedCount++;
            if (finishedCount == 4) {
                cv.notify_one();
            }
            // Wait until the generator creates new matrices (finishedCount resets to 0)
            cv.wait(lk, [] { return finishedCount == 0; });
        }
    }
}

// Function for generating new matrices
void matrixGeneratorThread() {
    while (true) {
        // Wait until all 4 multiplication threads complete the current cycle
        {
            unique_lock<mutex> lk(cv_mtx);
            cv.wait(lk, [] { return finishedCount == 4; });
        }

        // Double the matrix size
        currentN *= 2;
        int newSize = currentN;

        // Decorative output for generator activity
        cout << "\n---------------------------------------------------" << endl;
        cout << "Generator: creating new matrices of size " << newSize << "x" << newSize << endl;
        cout << "---------------------------------------------------" << endl;

        // Generate new matrices:
        // Matrix A filled with ones, Matrix B filled with twos
        vector<vector<int>> newA(newSize, vector<int>(newSize, 1));
        vector<vector<int>> newB(newSize, vector<int>(newSize, 2));

        // Update global matrices (within critical section)
        {
            unique_lock<mutex> lk(cv_mtx);
            globalA = move(newA);
            globalB = move(newB);
            // Reset counter to start a new cycle
            finishedCount = 0;
            // Notify all threads that new matrices are ready for multiplication
            cv.notify_all();
        }
    }
}

int main() {
    // Initialize global matrices of size currentN x currentN
    globalA = vector<vector<int>>(currentN, vector<int>(currentN, 1));
    globalB = vector<vector<int>>(currentN, vector<int>(currentN, 2));

    // Create 4 threads for matrix multiplication
    const int numMultiplicationThreads = 4;
    thread multThreads[numMultiplicationThreads];
    for (int i = 0; i < numMultiplicationThreads; ++i) {
        multThreads[i] = thread(multiplicationThread, i);
    }

    // Create a 5th thread for generating matrices
    thread generatorThread(matrixGeneratorThread);

    // Join threads (the program runs indefinitely)
    for (int i = 0; i < numMultiplicationThreads; ++i) {
        multThreads[i].join();
    }
    generatorThread.join();

    return 0;
}
