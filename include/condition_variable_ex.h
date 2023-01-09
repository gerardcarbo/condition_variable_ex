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

  std::ostream& operator<<(std::ostream& out, const cv_status_ex value){
    static std::map<cv_status_ex, std::string> strings;
    if (strings.size() == 0){
#define INSERT_ELEMENT(p) strings[p] = #p
        INSERT_ELEMENT(cv_status_ex::timeout);     
        INSERT_ELEMENT(cv_status_ex::signaled);     
        INSERT_ELEMENT(cv_status_ex::predicate);             
#undef INSERT_ELEMENT
    }   

    return out << strings[value];
}

  /// condition_variable
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
                            chrono::__detail::ceil<__dur>(__rtime), __p);
    }

    template <typename _Clock, typename _Duration, typename _Predicate>
    cv_status_ex
    wait_until_ex(unique_lock<mutex> &__lock,
               const chrono::time_point<_Clock, _Duration> &__atime,
               _Predicate __p)
    {
      while (!__p())
      {
        const auto __s_atime = __clock_t::now() + chrono::milliseconds(10);
        auto status = wait_until(__lock, __atime);
        cout << "wait done: " << status << " pred: " << __p() << endl;
        if(__p())
        {
          return cv_status_ex::predicate;
        }
        if (status == cv_status_ex::signaled)
        {
          return cv_status_ex::signaled;
        }
        if (__clock_t::now() >= __atime)
        {
          return cv_status_ex::timeout;
        }
      }
      return cv_status_ex::predicate;
    }

  private:
  #ifdef _GLIBCXX_USE_PTHREAD_COND_CLOCKWAIT
    template <typename _Dur>
    cv_status
    __wait_until_impl(unique_lock<mutex> &__lock,
                      const chrono::time_point<steady_clock, _Dur> &__atime)
    {
      auto __s = chrono::time_point_cast<chrono::seconds>(__atime);
      auto __ns = chrono::duration_cast<chrono::nanoseconds>(__atime - __s);

      __gthread_time_t __ts =
          {
              static_cast<std::time_t>(__s.time_since_epoch().count()),
              static_cast<long>(__ns.count())};

      _M_cond.wait_until(*__lock.mutex(), CLOCK_MONOTONIC, __ts);

      return (steady_clock::now() < __atime
                  ? cv_status::no_timeout
                  : cv_status::timeout);
    }
#endif

    template <typename _Dur>
    cv_status
    __wait_until_impl(unique_lock<mutex> &__lock,
                      const chrono::time_point<system_clock, _Dur> &__atime)
    {
      auto __s = chrono::time_point_cast<chrono::seconds>(__atime);
      auto __ns = chrono::duration_cast<chrono::nanoseconds>(__atime - __s);

      __gthread_time_t __ts =
          {
              static_cast<std::time_t>(__s.time_since_epoch().count()),
              static_cast<long>(__ns.count())};

      _M_cond.wait_until(*__lock.mutex(), __ts);

      return (system_clock::now() < __atime
                  ? cv_status::no_timeout
                  : cv_status::timeout);
    }

    template <typename _Clock, typename _Duration>
    cv_status_ex
    wait_until(unique_lock<mutex> &__lock,
               const chrono::time_point<_Clock, _Duration> &__atime)
    {
#if __cplusplus > 201703L
      static_assert(chrono::is_clock_v<_Clock>);
#endif
      using __s_dur = typename __clock_t::duration;
      const typename _Clock::time_point __c_entry = _Clock::now();
      const __clock_t::time_point __s_entry = __clock_t::now();
      const auto __delta = __atime - __c_entry;
      const auto __s_atime = __s_entry +
                             chrono::__detail::ceil<__s_dur>(__delta);

      if (__wait_until_impl(__lock, __s_atime) == cv_status::no_timeout)
        return cv_status_ex::signaled;
      // We got a timeout when measured against __clock_t but
      // we need to check against the caller-supplied clock
      // to tell whether we should return a timeout.
      if (_Clock::now() < __atime)
        return cv_status_ex::signaled;
      return cv_status_ex::timeout;
    }
  };
}