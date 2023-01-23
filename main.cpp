#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>
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

// working thread stuff
std::mutex m;
condition_variable_ex cv;
std::mutex m_done;
condition_variable_ex cv_done;
bool done = false;
bool ready = false;
bool processed = false;
cv_status_ex finalStatus;

// bgnd working thread test stuff
std::mutex m_working;
bool working = false;
condition_variable_ex cv_working;
bool started = false;
std::mutex m_started;
condition_variable_ex cv_started;
bool awake_thread = false;

// multithreaded protected cout
class CoutThread : public std::ostringstream
{
public:
    CoutThread() = default;

    ~CoutThread()
    {
        std::lock_guard<std::mutex> guard(_mutexPrint);
        cout << getCurrentTimestamp() << ": " << this->str();
    }

    std::string getCurrentTimestamp()
    {
        using std::chrono::system_clock;
        auto currentTime = std::chrono::system_clock::now();
        char buffer[120];

        auto transformed = currentTime.time_since_epoch().count() / 1000000;

        auto millis = transformed % 1000;

        std::time_t tt;
        tt = system_clock::to_time_t(currentTime);
        auto timeinfo = localtime(&tt);
        strftime(buffer, 120, "%H:%M:%S", timeinfo);
        sprintf(&(buffer[strlen(buffer)]), ":%03d", (int)millis);

        return std::string(buffer);
    }

private:
    static std::mutex _mutexPrint;
};
std::mutex CoutThread::_mutexPrint{};

// rand[0,1]
double randd()
{
    return (double)rand() / (double)RAND_MAX;
}

// test 1
void worker_thread(int timeout)
{
    ready = false;
    auto predicate = []
    { return ready; };

    // Wait until main() sends data
    std::unique_lock lk(m);
    finalStatus = cv.wait_for_ex(lk, chrono::milliseconds(timeout), predicate);
    ready = false;

    // after the wait, we own the lock.
    CoutThread{} << "Wait finished: " << finalStatus << "\n";

    // Send data back to main()
    processed = true;

    // Manual unlocking is done before notifying, to avoid waking up
    // the waiting thread only to block again (see notify_one for details)
    lk.unlock();
    cv_done.notify_one();
}

void signaler_thread(int timeout)
{
    CoutThread{} << "signaler waiting " << timeout << "ms\n";
    this_thread::sleep_for(chrono::milliseconds(timeout));
    cv.notify_all();
    CoutThread{} << "signaled\n";
}

void predicater_thread(int timeout)
{
    CoutThread{} << "predicater waiting " << timeout << "ms\n";
    this_thread::sleep_for(chrono::milliseconds(timeout));
    std::lock_guard lk(m);
    ready = true;
    CoutThread{} << "predicated\n";
}

// test 2
void bgnd_worker_thread(int timeout)
{
    // the predicate can depends on the main work
    auto predicate = []
    { return awake_thread; };

    // notify thread started
    CoutThread{} << "bgnd started" << endl;
    std::unique_lock lk(m_started);
    started = true;
    lk.unlock();
    cv_started.notify_all();

    // test distinct timeout relations
    auto th_timeout = timeout / 2;
    if (randd() > .5)
    {
        th_timeout = randd() * timeout;
    }

    std::unique_lock lkw(m_working);
    while (true)
    {
        // wait with timeout and predicate
        CoutThread{} << "bgnd sleeping for " << th_timeout << "ms " << endl;
        auto finalStatus = cv_working.wait_for_ex(lkw, chrono::milliseconds(th_timeout), predicate);
        if (finalStatus == cv_status_ex::signaled || !working)
        {
            CoutThread{} << "bgnd exitting thread. wait status: " << finalStatus << ". working: " << working << endl;
            break;
        }

        CoutThread{} << "bgnd continue working. wait status: " << finalStatus << ". working: " << working << endl;

        // do some work
        CoutThread{} << "bgnd working" << endl;
        this_thread::sleep_for(chrono::microseconds((int)(timeout / 2 * randd())));
        awake_thread = false;
    }
}

int main(int n, char **args)
{
    int type = atoi(args[1]);
    int repeat = atoi(args[2]);
    CoutThread{} << "test type: " << type << " num repeat: " << repeat << endl;
    if (type == 1)
    {
        cv_status_ex desiredStatus = (cv_status_ex)atoi(args[3]);
        int thread_timeout = atoi(args[4]);
        int signal_timeout = atoi(args[5]);
        int predicate_timeout = atoi(args[6]);

        CoutThread{} << "thread_timeout: " << thread_timeout << " signal_timeout: " << signal_timeout << " predicate_timeout: " << predicate_timeout << "\n";

        for (int i = 0; i < repeat; i++)
        {
            CoutThread{} << "******** " << i << " ********" << endl;
            /**
             * worker thread test.
             */
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

            if (desiredStatus != finalStatus)
            {
                CoutThread{} << "Failed: it should be " << desiredStatus << " and is " << finalStatus << "\n";
                return 1;
            }
            CoutThread{} << endl;
        }
        return 0;
    }
    else if (type == 2)
    {
        int loop_timeout = atoi(args[3]);

        /**
         * Background worker thread test.
         */

        for (int i = 0; i < repeat; i++)
        {
            CoutThread{} << "******** " << i << " ********" << endl;
            CoutThread{} << "loop_timeout: " << loop_timeout << endl;

            awake_thread = false;
            working = true;
            started = false;

            std::thread bgnd_worker(bgnd_worker_thread, loop_timeout);
            // wait for thread to be ready
            {
                CoutThread{} << "main program waithing thread ready" << endl;
                std::unique_lock lk(m_started);
                cv_started.wait(lk, [] { return started; });
            }

            // do some work
            int workTime = (int)(loop_timeout * (randd()));
            CoutThread{} << "main program do some work " << workTime << "ms " << endl;
            this_thread::sleep_for(chrono::milliseconds(workTime));
            if (randd() > .5)
            {
                // ..and awake the thread
                CoutThread{} << "main program change predicate" << endl;
                awake_thread = true;

                if (randd() > .5)
                {
                    // ..and some more work
                    workTime = (int)(1 * loop_timeout * (randd()));
                    CoutThread{} << "main program do some more work " << workTime << "ms " << endl;
                    this_thread::sleep_for(chrono::milliseconds(workTime));
                }
            }

            // stop and join thread
            CoutThread{} << "main program exiting" << endl;
            std::unique_lock<std::mutex> lck(m_working);
            working = false;
            lck.unlock();
            cv_working.notify_all();
            CoutThread{} << "main waiting thread" << endl;
            bgnd_worker.join();
            CoutThread{} << "joined " << i << endl
                         << endl;
        }
    }

    return 0;
};
