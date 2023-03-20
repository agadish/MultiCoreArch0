/**
 * @file parsort.cpp
 * @author Assaf Gadish
 * @purpose Multi-Core Architecture and Systems - Home Assignment 0
 */
/*   I N C L U D E S   */
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstdlib>
#include <chrono>
#include <algorithm>
#include <cilk/cilk.h>


#include "parsort.hpp"

using namespace std;


/*   E N U M S   */
enum parsort_arg_e {
    PARSORT_ARG_PROGRAM_NAME = 0,
    PARSORT_ARG_CORES,
    PARSORT_ARG_PATH,
    PARSORT_ARG_COUNT,
};

enum result_e {
    E__UNKNOWN = -1,
    E__SUCCESS = 0,
    E__INVALID_ARGS,
    E__INVALID_NUMBER_OF_CORES,
    E__INVALID_INPUT_FILE,
};



/*   F U N C T I O N S   */
template<typename T> void qsort(T begin, T end)
{
    if (begin != end) {
        T middle = partition(begin, end,
                       bind2nd(less<typename iterator_traits<T>::value_type>(),
                           *begin)); 

        cilk_spawn qsort(begin, middle);
        // INTENTIONAL RACE: MIDDLE-1 should be MIDDLE
        qsort(max(begin + 1, middle - 1), end);
        cilk_sync;
    }
}

Parsort::Parsort(int numberOfCores) : 
    mNumberOfCores(numberOfCores),
    mNumbers()
{
}

Parsort::~Parsort()
{
}

bool Parsort::read_input_file(const char *inputFilePath)
{
    mNumbers.clear();
    ifstream inputFile;
    inputFile.open(inputFilePath);
    if (!inputFile.is_open()) {
        return false;
    }

    uint64_t number = 0;
    while (inputFile >> number) {
        mNumbers.push_back(number);
    }

    return true;
}

void Parsort::sort()
{
    qsort(mNumbers.begin(), mNumbers.end());
}

void Parsort::print()
{
    for (auto i : mNumbers) {
        cout << i << endl;
    }
}

int main(int argc, const char *argv[])
{
    int number_of_cores = 0;
    const char *input_file_path = NULL;

    // 1. Parse input
    if (PARSORT_ARG_COUNT != argc) {
        cerr << "Usage: " << argv[0] << "NumberOfCores InputFile" << endl;
        return E__INVALID_ARGS;
    }

    number_of_cores = atoi(argv[PARSORT_ARG_CORES]);
    if (0 == number_of_cores) {
        cerr << "ERROR: Invalid number of cores must be >0" << endl;
        return E__INVALID_NUMBER_OF_CORES;
    }
    input_file_path = argv[PARSORT_ARG_PATH];
    
    // 2. Parse input file
    Parsort parsort(number_of_cores);
    bool result_read_input_file = parsort.read_input_file(input_file_path);
    if (!result_read_input_file) {
        cerr << "ERROR: Invalid input file" << endl;
        return E__INVALID_INPUT_FILE;
    }
    
    // 3. Sort
    auto start_time = chrono::high_resolution_clock::now();
    parsort.sort();
    auto end_time = chrono::high_resolution_clock::now();
    auto duration = end_time - start_time;

    // 3. Print result
    cout << "QuickSort: " << duration / chrono::milliseconds(1) << endl;
    parsort.print();

    return E__SUCCESS;
}
