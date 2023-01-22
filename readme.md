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

One tipical case is a backgroud worker thread, a perennial worker thread that can be activaded by the main thread or others on demand or by a timeout.

## How to compile

- As condition_variable_ex inherits from condition_variable, for the library to compile, the private members of condition_variable class must be declared protected. One modified copy is provided in the include folder.

- C++17 or greater needed.