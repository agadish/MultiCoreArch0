/**
 * @file parsort.hpp
 * @author Assaf Gadish
 * @purpose Multi-Core Architecture and Systems - Home Assignment 0
 *
 * @documentation The Parsort class works by reading uint64_t numbers from the text file, and
 *                dividing them to pre-allocated vectors of 512 members, in order for each
 *                vector to consume 4096 bytes and be placed on one page.
 *                All those pages are inserted to a global jobs list as SortJob which will
 *                sort each 512-members vector.
 *
 *                Workers:
 *                The total amount of data is then divided by the number of workers, and each
 *                worker repeatedly tries to take a greater/equal amount of data, and performs that
 *                job.
 *                After the job is done, the worker creates new jobs list as MergeJob and
 *                the worker will push it to the global list, before trying to pull new work for
 *                itself.
 *
 * @improvements A more sophisticated algorithm for the amount of work being taken can be used,
 *               since the push/pull game doesn't really help with the same amount
 *
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

/**
 * @brief A job has a callable function which performs calculations, and can be used to represent
 * simple task such as SortJob which sorts a vector or MergeJob that merges two sorted vectors
 */
class Job {
public:
    Job();
    ~Job();
    virtual datapage_t do_work() = 0;
    virtual size_t get_size() = 0;
};


/**
 * @brief A job that sorts a given vector
 */
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

/**
 * @brief A job that create a sorted vector given two sorted vectors by merging them
 */
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
    vector<shared_ptr<Job>> mJobs;

    /**
     * @brief Called by a worker in order to forwards jobs and optionally a single vector
     * (can be 0-sized) to the general pool in order for other workers to be able to catch it
     *
     * @param new_jobs New jobs to publish
     * @param last_result A single vector to publish
     *
     * @remark Must be called thread safely
     */
    void
    publish_results(const vector<shared_ptr<Job>> &new_jobs, const datapage_t &last_result);

    /**
     * @brief Gets a list of jobs and removes them from the general jobs list
     *
     * @return The jobs list
     *
     * @remark Must be called thread safely
     */
    vector<shared_ptr<Job>>
    get_jobs();

    /**
     * @brief The worker function that is executed in parallel in order to do jobs
     *
     * @return id The id of the worker
     */
    void
    sort__worker(size_t id);

public:
    Parsort(int numberOfCores);
    ~Parsort();

    /**
     * @brief Reads a given file that contains numbers and initialises the class's data
     *
     * @param inputFilePath the path to the file
     *
     * @return true if was read successfully, otherwise false
     */
    bool
    read_input_file(const char *inputFilePath);

    /**
     * @brief Sorts the data that was previously read through read_input_file.
     *
     * @remark Will invoke workers using OpenMP
     */
    void
    sort();

    /**
     * @brief Prints all the sorted numbers, and the sort duration in milliseconds
     */
    void
    print();
};

