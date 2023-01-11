#ifndef _GECAIB_CONDITION_VARIABLE_EX
#define _GECAIB_CONDITION_VARIABLE_EX 1

#include <condition_variable>
#include <map>

using namespace std;

namespace gecaib _GLIBCXX_VISIBILITY(default)
{
  enum class cv_status_ex
  {
    timeout,
    signaled,
    predicate
  };

  /// condition_variable_ex
  class condition_variable_ex : public condition_variable
  {
  public:
    condition_variable_ex(){};
    ~condition_variable_ex(){};

    condition_variable_ex(const condition_variable_ex &) = delete;
    condition_variable_ex &operator=(const condition_variable_ex &) = delete;

    template <typename _Rep, typename _Period, typename _Predicate>
    cv_status_ex
    wait_for_ex(unique_lock<mutex> &__lock,
                const chrono::duration<_Rep, _Period> &__rtime,
                _Predicate __p)
    {
      using __dur = typename steady_clock::duration;
      return wait_until_ex(__lock,
                           steady_clock::now() +
                               chrono::ceil<__dur>(__rtime),
                           __p);
    }

    template <typename _Clock, typename _Duration, typename _Predicate, typename _Period=std::milli>
    cv_status_ex
    wait_until_ex(unique_lock<mutex> &__lock,
                  const chrono::time_point<_Clock, _Duration> &__atime,
                  _Predicate __p,
                  const chrono::duration<int64_t,_Period> __pDuration=chrono::milliseconds(10))
    {
      while (!__p())
      {
        using __dur = typename steady_clock::duration;
        const auto __s_atime = steady_clock::now() +
                            chrono::ceil<__dur>(__pDuration) ;
        cv_status status = wait_until(__lock, __s_atime);
        //cout << "wait done: " << status << " pred: " << __p() << endl;
        if (__p())
        {
          return cv_status_ex::predicate;
        }
        if (status == cv_status::no_timeout)
        {
          return cv_status_ex::signaled;
        }
        if (steady_clock::now() >= __atime)
        {
          return cv_status_ex::timeout;
        }
      }
      return cv_status_ex::predicate;
    }
  };
}

#endif // _GECAIB_CONDITION_VARIABLE_EX
