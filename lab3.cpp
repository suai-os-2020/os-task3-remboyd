#include "lab3.h"
#include <windows.h>
#include <stdio.h>

#define NUMBER_OF_THREAD 11
#define NUMBER_OF_INTERVAL 6

using namespace std;

// thread identifiers
HANDLE aThread[NUMBER_OF_THREAD];
// semaphores for sequential threads
HANDLE semD, semE, semF, semH; // семафоры для потоков с чередованием
HANDLE mutex;
HANDLE interval[NUMBER_OF_INTERVAL];
HANDLE intervalDone;


unsigned int lab3_thread_graph_id()
{
    return 3;
}

const char* lab3_unsynchronized_threads()
{
    return "fgh";
}

const char* lab3_sequential_threads()
{
    return "defh";
}

void print_thread(int inum, const char *c) {
    WaitForSingleObject(interval[inum], INFINITE);
    for (int i = 0; i < 3; ++i) {
        WaitForSingleObject(mutex, INFINITE);
        cout << c << flush;
        ReleaseMutex(mutex);
        computation();
    }
    ReleaseSemaphore(intervalDone, 1, NULL);
}

void print_thread_sequential(int inum, const char *c, HANDLE &waiting, HANDLE &posting) {
    WaitForSingleObject(interval[inum], INFINITE);
    for (int i = 0; i < 3; ++i) {
        WaitForSingleObject(waiting, INFINITE);
        WaitForSingleObject(mutex, INFINITE);
        cout << c << flush;
        ReleaseMutex(mutex);
        computation();
        ReleaseSemaphore(posting, 1, nullptr);
    }
    ReleaseSemaphore(intervalDone, 1, NULL);
}

void create_thread(int thread_id, LPTHREAD_START_ROUTINE func) {
    aThread[thread_id] = CreateThread(nullptr, 0, func, nullptr, 0, nullptr);
    if (aThread[thread_id] == nullptr) {
        cerr << "Unable to create thread" << endl;
        exit(GetLastError());
    }
}

void interval_initializer(int inum) { // функция запуска интервалов (тут еще надо чет переделать с орг. сем)
    int len;
    switch (inum) {
        case 0: {
            len = 3;
            break;
        }
        case 1: {
            len = 3;
            break;
        }
        case 2: {
            len = 4;
            break;
        }
        case 3: {
            len = 3;
            break;
        }
        case 4: {
            len = 3;
            break;
        }
        case 5: {
            len = 2;
            break;
        }
        case 6: {
            return;
        }
        default: {
            cerr << "Invalid interval" << endl;
            exit(1);
        }

    }
    for (int i = 0; i < len; ++i) ReleaseSemaphore(interval[inum], 1, NULL);
    for (int i = 0; i < len; ++i) WaitForSingleObject(intervalDone, INFINITE);
    interval_initializer(++inum);
}

DWORD WINAPI thread_a(LPVOID pointer); ///a 0
DWORD WINAPI thread_b(LPVOID pointer); ///b 1
DWORD WINAPI thread_c(LPVOID pointer); ///c 2
DWORD WINAPI thread_d(LPVOID pointer); ///d 3
DWORD WINAPI thread_e(LPVOID pointer); ///e 4
DWORD WINAPI thread_f(LPVOID pointer); ///f 5
DWORD WINAPI thread_h(LPVOID pointer); ///h 6
DWORD WINAPI thread_g(LPVOID pointer); ///g 7
DWORD WINAPI thread_i(LPVOID pointer); ///i 8
DWORD WINAPI thread_k(LPVOID pointer); ///k 9
DWORD WINAPI thread_m(LPVOID pointer); ///m 10

DWORD WINAPI thread_a(LPVOID pointer) {
    print_thread(0, "a");
    create_thread(2, thread_c);
    return 0;
}


DWORD WINAPI thread_b(LPVOID pointer) {
    print_thread(0, "b");
    print_thread(1, "b");
    create_thread(4, thread_e);
    create_thread(5, thread_f);
    create_thread(6, thread_h);
    WaitForSingleObject(aThread[1], INFINITE); /// ждем завершения B
    return 0;
}

DWORD WINAPI thread_c(LPVOID pointer) {
    print_thread(1, "c");
    return 0;
}

DWORD WINAPI thread_d(LPVOID pointer) {
    print_thread(0, "d");
    print_thread(1, "d");
    print_thread_sequential(2, "d", semD, semE);
    create_thread(7, thread_g);
    return 0;
}

DWORD WINAPI thread_e(LPVOID pointer) {
    print_thread_sequential(2, "e", semE, semF);
    return 0;
}

DWORD WINAPI thread_f(LPVOID pointer) {
    print_thread_sequential(2, "f", semF, semH);
    print_thread(3, "f");
    return 0;
}

DWORD WINAPI thread_h(LPVOID pointer) {
    print_thread_sequential(2, "h", semH, semD);
    print_thread(3, "h");
    print_thread(4, "h");
    create_thread(10, thread_m);
    WaitForSingleObject(aThread[6], INFINITE); /// ждем завершения H
    return 0;
}

DWORD WINAPI thread_g(LPVOID pointer) {
    print_thread(3, "g");
    create_thread(8, thread_i);
    create_thread(9, thread_k);
    return 0;
}

DWORD WINAPI thread_i(LPVOID pointer) {
    print_thread(4, "i");
    return 0;
}

DWORD WINAPI thread_k(LPVOID pointer) {
    print_thread(4, "k");
    print_thread(5, "k");
    return 0;
}

DWORD WINAPI thread_m(LPVOID pointer) {
    print_thread(5, "m");
    WaitForSingleObject(aThread[10], INFINITE); /// ждем завершения M
    return 0;
}

int lab3_init() {
    // initialize mutex
    mutex = CreateMutex(NULL, FALSE, NULL);
    if (mutex == nullptr) {
        cerr << "Mutex initialize failed" << endl;
        return GetLastError();
    }

    intervalDone = CreateSemaphore(NULL,0,1,NULL);
    if (intervalDone == nullptr) {
        printf("Unable to create semaphore");
        return GetLastError();
    }
    for(int i = 0; i < NUMBER_OF_INTERVAL; i++) {
        interval[i] = CreateSemaphore(NULL, 0, 10, NULL);
        if (interval[i] == nullptr) {
            printf("Unable to create semaphore");
            return GetLastError();
        }
    }

    // initialize semaphores
    semD = CreateSemaphore(NULL, 1, 1, NULL);
    semE = CreateSemaphore(NULL, 0, 1, NULL);
    semF = CreateSemaphore(NULL, 0, 1, NULL);
    semH = CreateSemaphore(NULL, 0, 1, NULL);

    if (semD == nullptr || semE == nullptr || semF == nullptr || semH == nullptr) {
        printf("Unable to create semaphore");
        return GetLastError();
    }

    create_thread(0, thread_a);
    create_thread(1, thread_b);
    create_thread(3, thread_d);
    interval_initializer(0);
    WaitForSingleObject(aThread[0], INFINITE);
    CloseHandle(mutex);
    CloseHandle(semD);
    CloseHandle(semE);
    CloseHandle(semF);
    CloseHandle(semH);
    return 0;
}