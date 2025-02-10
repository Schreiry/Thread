// std
using namespace std;

// -==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// Directive Sapce 

#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>
#include <vector>

// -==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


// Глобальные переменные для матриц и их размера
int currentN = 10; // Начальный размер матриц
vector<vector<int>> globalA;
vector<vector<int>> globalB;

// Мьютекс и условная переменная для синхронизации
mutex cv_mtx;
condition_variable cv;
int finishedCount = 0; // Счётчик завершённых потоков умножения

// Функция умножения матриц (стандартная реализация)
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

// Функция, выполняемая в потоках умножения матриц
void multiplicationThread(int threadId) {
    while (true) {
        // Считываем текущий размер матриц (это целое, поэтому здесь не нужен дополнительный mutex)
        int N_local = currentN;

        // Локальный результат умножения
        vector<vector<int>> C(N_local, vector<int>(N_local, 0));

        // Замер времени начала умножения
        auto start = chrono::high_resolution_clock::now();

        // Выполняем умножение глобальных матриц (предполагается, что обновление происходит только между циклами)
        multiplyMatrices(globalA, globalB, C, N_local);

        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double> duration = end - start;


        cout << "=============================================" << endl;
        cout << ">>> Thread " << threadId << " completed matrix multiplication" << endl;
        cout << "Matrix size : " << N_local << "x" << N_local << endl;
        cout << "lead time : [" << duration.count() << " seconds]" << endl;
        cout << "=============================================" << endl;

        // Синхронизация: сообщаем, что данный поток завершил цикл умножения
        {
            unique_lock<mutex> lk(cv_mtx);
            finishedCount++;
            // Если все 4 потока завершили умножение, уведомляем генератор
            if (finishedCount == 4) {
                cv.notify_one();
            }
            // Ожидаем, пока генератор не создаст новые матрицы (finishedCount сбрасывается в 0)
            cv.wait(lk, [] { return finishedCount == 0; });
        }
    }
}

// Функция генератора новых матриц
void matrixGeneratorThread() {
    while (true) {
        // Ждём, пока все 4 потока умножения не завершат текущий цикл
        {
            unique_lock<mutex> lk(cv_mtx);
            cv.wait(lk, [] { return finishedCount == 4; });
        }

        // Обновляем размер матриц: удваиваем
        currentN *= 2;
        int newSize = currentN;


        cout << "\n---------------------------------------------------" << endl;
        cout << "Generator: creating new matrices of size " << newSize << "x" << newSize << endl;
		cout << "---------------------------------------------------" << endl;




        // Генерация новых матриц:
        // Матрица A заполняется единицами, B – двойками (можно изменить логику генерации по необходимости)
        vector<vector<int>> newA(newSize, vector<int>(newSize, 1));
        vector<vector<int>> newB(newSize, vector<int>(newSize, 2));

        // Обновляем глобальные матрицы (в критической секции)
        {
            unique_lock<mutex> lk(cv_mtx);
            globalA = move(newA);
            globalB = move(newB);

            // Сбрасываем счётчик, чтобы начать новый цикл
            finishedCount = 0;
            // Уведомляем все потоки о том, что новые матрицы готовы и можно приступать к умножению
            cv.notify_all();
        }
    }
}

int main() {
    // Инициализируем глобальные матрицы размером 500x500
    globalA = vector<vector<int>>(currentN, vector<int>(currentN, 1));
    globalB = vector<vector<int>>(currentN, vector<int>(currentN, 2));

    // Создаём 4 потока для умножения матриц
    const int numMultiplicationThreads = 4;
    thread multThreads[numMultiplicationThreads];
    for (int i = 0; i < numMultiplicationThreads; ++i) {
        multThreads[i] = thread(multiplicationThread, i);
    }

    // Создаём 5-ый поток для генерации матриц
    thread generatorThread(matrixGeneratorThread);

    // Присоединяем потоки (хотя программа работает вечно)
    for (int i = 0; i < numMultiplicationThreads; ++i) {
        multThreads[i].join();
    }
    generatorThread.join();

    return 0;
}