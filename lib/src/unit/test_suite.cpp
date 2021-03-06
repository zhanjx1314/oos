#include "unit/test_suite.hpp"
#include "unit/unit_test.hpp"

#include <algorithm>
#include <functional>
#include <iostream>

namespace oos {

test_suite::test_suite()
{}

void test_suite::register_unit(unit_test *utest)
{
  unit_test_map_.insert(std::make_pair(utest->name(), unit_test_ptr(utest)));
}

test_suite::unit_executer::unit_executer()
  : succeeded(true)
{}

void test_suite::unit_executer::operator()(test_suite::value_type &x)
{
  std::cout << "Executing test unit [" << x.second->caption() << "]\n";
  bool result = x.second->execute();
  if (succeeded && !result) {
    succeeded = result;
  }
}

test_suite::unit_lister::unit_lister(std::ostream &o, bool b)
  : out(o), brief(b)
{}

void test_suite::unit_lister::operator()(const test_suite::value_type &x) const
{
  if (!brief) {
    out << "Unit Test [" << x.first << "] has the following test:\n";
  }

  const unit_test::t_test_func_info_vector &tests = x.second->test_func_infos_;
  std::for_each(tests.begin(), tests.end(), [this](const unit_test::t_test_func_info_vector::value_type &x) {
    if (brief) {
      out << x.name << ":" << x.caption << "\n";
    } else {
      out << "Test [" << x.name << "]: " << x.caption << std::endl;
    }
  });
}

test_suite::~test_suite()
{
  unit_test_map_.clear();
}

void test_suite::init(int argc, char *argv[])
{
  // check for
  if (argc < 2) {
    // no arguments
    return;
  }
  std::string arg(argv[1]);
  
  if (arg == "list") {
    // list all test units
    args_.cmd = LIST;
    if (argc == 3) {
      if (strcmp(argv[2], "brief") == 0) {
        args_.brief = true;
      } else {
        // unknow option arg
      }
    } else {
    }
  } else if (arg == "exec") {
    if (argc < 3) {
      return;
    }

    args_.cmd = EXECUTE;

    std::string val(argv[2]);
    if (val == "all") {
    } else {
      std::stringstream sval(val);
      std::string part;
      while (std::getline(sval, part, ',')) {
        size_t pos = part.find(':');

        test_unit_args unit_args;

        if (pos == std::string::npos) {
          // execute all test of a unit class
          unit_args.unit = part;
        } else {
          // extract unit and test name
          unit_args.unit = part.substr(0, pos);
          std::stringstream tests(part.substr(pos+1));
          std::string test;
          while (std::getline(tests, test, ':')) {
            unit_args.tests.push_back(test);
          }
        }

        args_.unit_args.push_back(unit_args);
      }
    }
  } else {
    return;
  }
  args_.initialized = true;
}

bool test_suite::run()
{
  if (args_.initialized) {
    switch (args_.cmd) {
      case LIST:
        std::for_each(unit_test_map_.begin(), unit_test_map_.end(), unit_lister(std::cout, args_.brief));
        break;
      case EXECUTE:
        if (!args_.unit_args.empty()) {
          bool result = true;
          for (auto item : args_.unit_args) {
            bool succeeded = run(item);
            if (result && !succeeded) {
              result = succeeded;
            }
          }
          return result;
        } else {
          unit_executer ue;
          std::for_each(unit_test_map_.begin(), unit_test_map_.end(), ue);
          return ue.succeeded;
        }
      default:
        break;
      }
  } else {
    std::cout << "usage: test_oos [list]|[exec <val>]\n";
  }
  return true;
}

bool test_suite::run(const std::string &unit)
{
  t_unit_test_map::const_iterator i = unit_test_map_.find(unit);
  if (i == unit_test_map_.end()) {
    std::cout << "couldn't find test unit [" << unit << "]\n";
    return false;
  } else {
    std::cout << "Executing test unit [" << i->second->caption() << "]\n";
    return i->second->execute();
  }
}

bool test_suite::run(const test_unit_args &unit_args)
{
  bool result = true;
  if (unit_args.tests.empty()) {
    result = run(unit_args.unit);
  } else {
    for (auto test : unit_args.tests) {
      bool succeeded = run(unit_args.unit, test);
      if (result && !succeeded) {
        result = succeeded;
      }
    }
  }
  return result;
}

bool test_suite::run(const std::string &unit, const std::string &test)
{
  t_unit_test_map::const_iterator i = unit_test_map_.find(unit);
  if (i == unit_test_map_.end()) {
    std::cout << "couldn't find test unit [" << unit << "]\n";
    return false;
  } else {
    return i->second->execute(test);
  }
}

}
