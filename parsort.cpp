/**
 * @file parsort.cpp
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
/*   I N C L U D E S   */
#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstdlib>
#include <chrono>
#include <algorithm>
#include <omp.h>

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
    E__SUCCESS = 0,
    E__INVALID_ARGS,
    E__INVALID_NUMBER_OF_CORES,
    E__INVALID_INPUT_FILE,
};



/*   F U N C T I O N S   */
Job::Job() { }
Job::~Job() { }

SortJob::SortJob(datapage_t job) :
    mJob(job)
{
}

SortJob::~SortJob() { }

datapage_t
SortJob::do_work()
{
    std::sort(mJob.begin(), mJob.end());
    return mJob;
}

size_t
SortJob::get_size()
{
    return mJob.size();
}

MergerJob::MergerJob(datapage_t a, datapage_t b) :
    mJobA(a),
    mJobB(b)
{
}

MergerJob::~MergerJob()
{
}

datapage_t
MergerJob::do_work()
{
    datapage_t result;
    result.reserve(mJobA.size() + mJobB.size());
    std::merge(mJobA.begin(), mJobA.end(),
               mJobB.begin(), mJobB.end(),
               std::back_inserter(result));

    return result;
}

size_t
MergerJob::get_size()
{
    return mJobA.size() + mJobA.size();
}

void
Parsort::publish_results(const vector<shared_ptr<Job>> &new_jobs, const datapage_t &last_result)
{
    // 1. Publish ready jobs
    copy(new_jobs.begin(), new_jobs.end(), back_inserter(mJobs));

    // 2. Save last result
    if (0 != last_result.size()) {
        if (0 == mDatapageLeftover.size()) {
            mDatapageLeftover = last_result;
        } else {
            shared_ptr<Job> new_job(new MergerJob(mDatapageLeftover, last_result));
            mJobs.push_back(new_job);
            mDatapageLeftover.clear();
        }
    } else {
        if (0 == mDatapageLeftover.size()) {
            mDatapageLeftover = last_result;
        }
    }
}

vector<shared_ptr<Job>>
Parsort::get_jobs()
{
    vector<shared_ptr<Job>> my_jobs;
    size_t total_work = 0;
    size_t desired_work = (mTotalNumbers + mNumberOfCores - 1) / mNumberOfCores;
    size_t copy_length = 0;

    for (auto job : mJobs) {
        ++copy_length;
        total_work += job->get_size();
        if (total_work > desired_work) {
            break;
        }
    }
    my_jobs.insert(my_jobs.end(),
                   std::make_move_iterator(mJobs.begin()),
                   std::make_move_iterator(mJobs.begin() + copy_length));
    mJobs.erase(mJobs.begin(), mJobs.begin() + copy_length);

    return my_jobs;
}

void
Parsort::sort__worker(size_t id)
{
    vector<shared_ptr<Job>> jobs;
    #pragma omp critical
    {
        jobs = get_jobs();
    }

    while (0 != jobs.size()) {
        vector<shared_ptr<Job>> new_jobs;
        datapage_t last_result;
        for (auto job : jobs) {
            auto job_result = job->do_work();
            if (0 != last_result.size()) {
                shared_ptr<Job> new_job(new MergerJob(last_result, job_result));
                new_jobs.push_back(new_job);
                last_result.clear();
            } else {
                last_result = job_result;
            }
        }

        #pragma omp critical
        {
            publish_results(new_jobs, last_result);
            jobs = get_jobs();
        }

    }
}

Parsort::Parsort(int numberOfCores) :
    mNumberOfCores(numberOfCores),
    mTotalNumbers(0),
    mDatapageLeftover(),
    mJobs()
{
    mPageSize = sysconf(_SC_PAGESIZE);
    mVarsPerPage = mPageSize / sizeof(uint64_t);
}

Parsort::~Parsort() { }

bool
Parsort::read_input_file(const char *inputFilePath)
{
    mJobs.clear();
    ifstream inputFile;
    inputFile.open(inputFilePath);
    if (!inputFile.is_open()) {
        return false;
    }

    mTotalNumbers = 0;
    uint64_t number = 0;
    bool has_input = true;
    while (has_input) {
        datapage_t curr_datapage;
		// Assuming the allocator alloactes a complete memory page
        curr_datapage.reserve(mVarsPerPage);
        for (size_t i = 0 ; i < mVarsPerPage ; ++i) {
            has_input = static_cast<bool>(inputFile >> number);
            if (!has_input) {
                // Input is over
                break;
            }
            ++mTotalNumbers;

            curr_datapage.push_back(number);
        }
        if (0 != curr_datapage.size()) {
            shared_ptr<Job> new_job(new SortJob(curr_datapage));
            mJobs.push_back(new_job);
        }
    }

    return true;
}

void
Parsort::sort()
{
    omp_set_num_threads(mNumberOfCores);
#pragma omp parallel
    {
        int id = omp_get_thread_num();
        sort__worker(static_cast<size_t>(id));
    }
}

void
Parsort::print()
{
    for (auto i : mDatapageLeftover) {
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

    // 4. Print result
    cout << "QuickSort: " << duration / chrono::milliseconds(1) << endl;
    parsort.print();

    return E__SUCCESS;
}
