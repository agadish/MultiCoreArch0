/**
 * @file parsort.hpp
 * @author Assaf Gadish
 * @purpose Parsort class definition
 */
#pragma once
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <memory>
#include <unistd.h>

using namespace std;

typedef std::vector<uint64_t> datapage_t;

class Job {
public:
    Job();
    ~Job();
    virtual datapage_t do_work() = 0;
    virtual size_t get_size() = 0;
};

class SortJob : public Job {
protected:
    datapage_t mJob;
public:
    SortJob(datapage_t job);

    ~SortJob();

    virtual datapage_t
    do_work();

    virtual size_t
    get_size();
};

class MergerJob : public Job {
protected:
    datapage_t mJobA;
    datapage_t mJobB;

public:
    MergerJob(datapage_t a, datapage_t b);

    ~MergerJob();

    virtual datapage_t
    do_work();

    virtual size_t
    get_size();
};


class Parsort {
protected:


    size_t mPageSize;
    size_t mVarsPerPage;

    int mNumberOfCores;
    size_t mTotalNumbers;

    datapage_t mDatapageLeftover;
    // omp_lock_t mDatapagesLock;
    vector<shared_ptr<Job>> mJobs;
    // omp_lock_t mJobsLock;


    void
    publish_results(const vector<shared_ptr<Job>> &new_jobs, const datapage_t &last_result);

    vector<shared_ptr<Job>>
    get_jobs();

    void
    sort__worker(size_t id);


public:
    Parsort(int numberOfCores);

    ~Parsort();

    bool
    read_input_file(const char *inputFilePath);

    void
    sort();

    void
    print();
};

