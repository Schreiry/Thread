
// --- Directive Space : ---
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <sstream>
#include <vector>

// ----  -----  -----

using std::mutex;
using std::vector;
using std::string;

// ----  -----  -----




// :::::::::Colors ::::::::::
const string RED = "\033[1;31m";
const string GREEN = "\033[1;32m";
const string YELLOW = "\033[1;33m";
const string BLUE = "\033[1;34m";
const string PURPLE = "\033[1;35m";
const string CYAN = "\033[1;36m";
const string RESET = "\033[0m";
// ::::::::: ---- ::::::::::
//|
// Global variables for matrices and their size
int currentN = 5; // Initial size of matrices
vector < vector < int >> globalA;
vector < vector < int >> globalB;
//|
// Mutex and condition variable for synchronizing matrix threads
mutex cv_mtx;
std::condition_variable cv;
int finishedCount = 0; // Counter of threads that have completed multiplication
//|
// Thread-safe queue for console output
std::queue<std::string> consoleQueue;
mutex consoleMutex;
std::condition_variable consoleCV;





// Function to safely place a message in the output queue
void pushConsoleMessage( const std::string& msg ) {
    {
        std::lock_guard<mutex> lock( consoleMutex );
        consoleQueue.push( msg );
    }
    consoleCV.notify_one( );
}

// Thread responsible for outputting messages from the queue to the console
void consoleThreadFunction( ) {
    while ( true ) {
        std::unique_lock<mutex> lock(consoleMutex);
        consoleCV.wait(lock, [] { return !consoleQueue.empty( ); });
        while ( !consoleQueue.empty( ) ) {
            std::string msg = consoleQueue.front( );
            consoleQueue.pop( );
            // Unlock the mutex while outputting so other threads can put new messages
            lock.unlock( );
            std::cout << msg;
            lock.lock( );
        }
    }
}

// Standard matrix multiplication function
void multiplyMatrices( const vector < vector < int >> & A,
    const vector<vector<int>>& B,
    vector<vector<int>>& C, int N ) {
    for ( int i = 0; i < N; ++i )
        for ( int j = 0; j < N; ++j ) {
            int sum = 0;
            for ( int k = 0; k < N; ++k )
                sum += A[i][k] * B[k][j];
            C[i][j] = sum;
        }
}

void multiplicationThread( int threadId ) {
    while ( true ) {
        int N_local = currentN;
        vector < vector < int >> C(N_local, vector < int >(N_local, 0));

        auto start = std::chrono::high_resolution_clock::now();
        multiplyMatrices(globalA, globalB, C, N_local);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;

        // Form a string with the result of the thread's work
        std::stringstream ss;
        ss << "\n"
            << BLUE << "::::::::::::::::::::::::::::::::" << RESET << "\n"
            << RED << "Thread: " << threadId << YELLOW << " completed matrix multiplication" << RESET << "\n"
            << CYAN << "Matrix size: " << N_local << "x" << N_local << RESET << "\n"
            << GREEN << "lead time: [" << duration.count() << " seconds ]" << RESET << "\n"
            << BLUE << "::::::::::::::::::::::::::::::::" << RESET << "\n\n";
        pushConsoleMessage(ss.str());

        // Synchronization with matrix generator
        {
            std::unique_lock<mutex> lk(cv_mtx);
            finishedCount++;
            if (finishedCount == 4) {
                cv.notify_one();
            }
            cv.wait(lk, [] { return finishedCount == 0; });
        }
    }
}

// Stream-generator of new matrices
void matrixGeneratorThread() {
    while (true) {
        {
            std::unique_lock<mutex> lk(cv_mtx);
            cv.wait(lk, [] { return finishedCount == 4; });
        }

        currentN *= 2;
        int newSize = currentN;

        std::stringstream ss;
        ss << YELLOW << "\n---------------------------------------------------" << RESET << "\n"
            << GREEN << "Generator: Create new matrices of size " << newSize << "x" << newSize << RESET << "\n"
            << YELLOW << "---------------------------------------------------" << RESET << "\n";
        pushConsoleMessage(ss.str());

        // Generate new matrices: A filled with ones, B with twos
        vector < vector < int >> newA( newSize, vector<int>( newSize, 1 ) );
        vector < vector < int >> newB( newSize, vector<int>( newSize, 2 ) );

        {
            std::unique_lock<mutex> lk(cv_mtx);
            globalA = std::move(newA);
            globalB = std::move(newB);
            finishedCount = 0;
            cv.notify_all();
        }
    }
}

int main() {
// Initialization of global matrices
    globalA = vector<vector<int>>(currentN, vector<int>(currentN, 1));
    globalB = vector<vector<int>>(currentN, vector<int>(currentN, 2));

// Create 4 threads for matrix multiplication
    const int numMultiplicationThreads = 4;
    std::thread multThreads[numMultiplicationThreads];
    for (int i = 0; i < numMultiplicationThreads; ++i) {
        multThreads[i] = std::thread(multiplicationThread, i);
    }

// Matrix Generator Stream
    std::thread generatorThread(matrixGeneratorThread);

// Stream for rendering output to console
    std::thread consoleThread(consoleThreadFunction);

// Threads run indefinitely, but can be joined to terminate gracefully
    for (int i = 0; i < numMultiplicationThreads; ++i) {
        multThreads[i].join();
    }
    generatorThread.join();
    consoleThread.join();

    return 0;
}
