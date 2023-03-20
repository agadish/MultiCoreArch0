/**
 * @file parsort.hpp
 * @author Assaf Gadish
 * @purpose Parsort class definition
 */
#pragma once
#include <cstdint>
#include <vector>

using namespace std;

class Parsort {
protected:
    int mNumberOfCores;
    vector<uint64_t> mNumbers;

public:
    Parsort(int numberOfCores);
    ~Parsort();
    bool read_input_file(const char *inputFilePath);
    void sort();
    void print();
};
