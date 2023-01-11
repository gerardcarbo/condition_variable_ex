#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable_ex.h>

using namespace gecaib;

std::ostream &operator<<(std::ostream &out, const cv_status_ex value)
{
static std::map<cv_status_ex, std::string> strings;
if (strings.size() == 0)
{
#define INSERT_ELEMENT(p) strings[p] = #p
    INSERT_ELEMENT(cv_status_ex::timeout);
    INSERT_ELEMENT(cv_status_ex::signaled);
    INSERT_ELEMENT(cv_status_ex::predicate);
#undef INSERT_ELEMENT
}

return out << strings[value];
}

std::ostream &operator<<(std::ostream &out, const cv_status value)
{
static std::map<cv_status, std::string> strings;
if (strings.size() == 0)
{
#define INSERT_ELEMENT(p) strings[p] = #p
    INSERT_ELEMENT(cv_status::timeout);
    INSERT_ELEMENT(cv_status::no_timeout);
#undef INSERT_ELEMENT
}

return out << strings[value];
}

std::mutex m;
condition_variable_ex cv;
std::mutex m_done;
condition_variable_ex cv_done;
bool ready = false;
bool processed = false;
cv_status_ex finalStatus;

void worker_thread(int timeout)
{
    // Wait until main() sends data
    std::unique_lock lk(m);
    finalStatus = cv.wait_for_ex(lk, chrono::milliseconds(timeout), []{ return ready; });

    // after the wait, we own the lock.
    std::cerr << "Wait finished: " << finalStatus << "\n";

    // Send data back to main()
    processed = true;

    // Manual unlocking is done before notifying, to avoid waking up
    // the waiting thread only to block again (see notify_one for details)
    lk.unlock();
    cv_done.notify_one();
}

void signaler_thread(int timeout)
{
    this_thread::sleep_for(chrono::milliseconds(timeout));
    cv.notify_all();
    std::cout << "signaled\n";
}

void predicater_thread(int timeout)
{
    this_thread::sleep_for(chrono::milliseconds(timeout));
    std::lock_guard lk(m);
    ready = true;
    std::cout << "predicated\n";
}

int main(int n, char **args)
{
    cv_status_ex desiredStatus = (cv_status_ex)atoi(args[1]);
    int thread_timeout = atoi(args[2]);
    int signal_timeout = atoi(args[3]);
    int predicate_timeout = atoi(args[4]);

    cout << "thread_timeout: " << thread_timeout << " signal_timeout: " << signal_timeout << " predicate_timeout: " << predicate_timeout << "\n";

    std::thread worker(worker_thread, thread_timeout);
    std::thread signaler(signaler_thread, signal_timeout);
    std::thread predicater(predicater_thread, predicate_timeout);

    // wait for the worker
    {
        std::unique_lock lk(m_done);
        cv_done.wait(lk, []
                { return processed; });
    }
    
    worker.join();
    signaler.join();
    predicater.join();

    if(desiredStatus == finalStatus){
        return 0;
    }
    cout << "Failed: it should be " << desiredStatus << " and is " << finalStatus << "\n";
    return 1;
};
