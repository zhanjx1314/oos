#ifndef TIME_HPP
#define TIME_HPP

#ifdef _MSC_VER
  #ifdef oos_EXPORTS
    #define OOS_API __declspec(dllexport)
    #define EXPIMP_TEMPLATE
  #else
    #define OOS_API __declspec(dllimport)
    #define EXPIMP_TEMPLATE extern
  #endif
  #pragma warning(disable: 4251)
#else
#define OOS_API
#endif

#include "tools/date.hpp"

#include <ctime>
#include <cstdint>
#include <string>

namespace oos {

class OOS_API time
{
public:
  enum tz_t {
    local,
    utc
  };

  static const char *default_format;

  /**
   * Creates a new time with
   * time now
   */
  time();

  explicit time(time_t t);
  explicit time(struct timeval tv);
  time(int year, int month, int day, int hour, int min, int sec, long millis = 0);
//  explicit time(uint64_t microseconds);
  time(const time &x);
  time &operator=(const time &x);
  ~time();

  bool operator==(const time &x) const;
  bool operator!=(const time &x) const;
  bool operator<=(const time &x) const;
  bool operator<(const time &x) const;
  bool operator>(const time &x) const;
  bool operator>=(const time &x) const;

  static time now();

  static bool is_valid_time(int hour, int min, int sec, long millis);

  static time parse(const std::string &tstr, const char *format);

  void set(int year, int month, int day, int hour, int min, int sec, long millis);
  void set(time_t t, long millis);
  void set(const date &d);
  void set(timeval tv);

//  std::string str() const;
//
//  std::string to_local(const char *f) const;
//
//  std::string to_utc(const char *f) const;

  int year() const;
  int month() const;
  int day() const;
  int hour() const;
  int minute() const;
  int second() const;
  int milli_second() const;

  time& year(int y);
  time& month(int m);
  time& day(int d);
  time& hour(int h);
  time& minute(int m);
  time& second(int s);
  time& milli_second(int ms);

  int day_of_week() const;
  int day_of_year() const;

  bool is_leapyear() const;
  bool is_daylight_saving() const;

  struct timeval get_timeval() const;
  struct tm get_tm() const;

  date to_date() const;

//  std::string format(const char *f, tz_t tz = local) const;
//  void parse(const char *f, const std::string &ts);

  friend std::ostream &operator<<(std::ostream &out, const time &x);
//  friend std::istream &operator>>(std::istream &in, time &x);

private:
  void sync_day(int d);
  void sync_month(int m);
  void sync_year(int y);
  void sync_hour(int h);
  void sync_minute(int m);
  void sync_second(int s);
  void sync_milli_second(int ms);
  void sync_time(int y, int m, int d, int h, int min, int s, long ms);

private:
  struct timeval time_;
  struct tm tm_;
};

}

#endif /* TIME_HPP */