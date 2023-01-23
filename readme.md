# condition_variable_ex

Adds to the condition_variable standard class the following methods:

- cv_status_ex `wait_for_ex`(unique_lock<mutex> &__lock,
                            const chrono::duration<_Rep, _Period> &__rtime,
                            _Predicate __p)

- cv_status_ex `wait_until_ex`(unique_lock<mutex> &__lock,
                            const chrono::time_point<_Clock, _Duration> &__atime,
                            _Predicate __p,
                            const chrono::duration<int64_t,_Period> __pDuration=chrono::milliseconds(10))
                        
Where `cv_status_ex` can be:

- `timeout`: wait abandoned by timeout.
- `signaled`: wait abandoned by singal (notify_one, notify_all).
- `predicate`: wait abandoned by predicate.

The method wait_until_ex has an additional `_pDuration` parameter which indicates how often the predicate must be inspected.

## Motivation

Sometimes it is interesting to know if the wait has been abandoned because of the signal or because of the predicate. With the standard class it is no possible, as it only returns timeout/no_timeout.

One tipical case is a backgroud worker thread, a perennial worker thread that can be activaded by the main thread or others on demand or by a timeout:

    std::mutex m_working;
    bool working = false;
    condition_variable_ex cv_working;
    bool awake_thread = false;

    void bgnd_worker_thread(int timeout)
    {
        // the predicate can depends on the main work
        auto predicate = []
        { return awake_thread; };

        std::unique_lock lkw(m_working);
        while (true)
        {
            // wait with timeout and predicate
            CoutThread{} << "bgnd sleeping for " << th_timeout << "ms " << endl;
            auto finalStatus = cv_working.wait_for_ex(lkw, chrono::milliseconds(th_timeout), predicate);
            if (finalStatus == cv_status_ex::signaled || !working)
            {
                break;
            }
            
            // do some work
            ...

            // reset the predicate
            awake_thread = false;
        }
    }

    void main() 
    {
        // start bgnd working thread
        awake_thread = false;
        working = true;
        std::thread bgnd_worker(bgnd_worker_thread, loop_timeout);

        ....

        // activate bgnd thread
        awake_thread = true;

        ....

        // stop and wait bgnd thread
        std::unique_lock<std::mutex> lck(m_working);
        working = false;
        lck.unlock();
        cv_working.notify_all();
        bgnd_worker.join();

        return;
    }


## How to compile

- As condition_variable_ex inherits from condition_variable, for the library to compile, the private members of condition_variable class must be declared protected. One modified copy is provided in the include folder.

- C++17 or greater needed.