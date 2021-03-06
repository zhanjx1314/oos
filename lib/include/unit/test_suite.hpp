/*
 * This file is part of OpenObjectStore OOS.
 *
 * OpenObjectStore OOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenObjectStore OOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenObjectStore OOS. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TEST_SUITE_HPP
#define TEST_SUITE_HPP

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

#include "tools/singleton.hpp"

#include <memory>
#include <map>
#include <string>
#include <vector>

/**
 * @namespace oos
 * @brief The Open Object Store namespace.
 *
 * This is the namespace where all OOS related classes,
 * function and code should be put into.
 */
namespace oos {

class unit_test;

/**
 * @class test_suite
 * @brief The container for all unit tests.
 * 
 * The test_suite class is the container for all test_unit
 * classes. It is a singleton, so only one instance exists.
 * It can execute all test_units or a specific test_unit.
 * It also provides function listing all test_unit classes
 * and their tests.
 */
class OOS_API test_suite : public singleton<test_suite>
{
private:
  friend class singleton<test_suite>;

  /**
   * @brief test_suite commands
   * 
   * This is an enumeration for
   * all test_suite commands
   */
  typedef enum test_suite_cmd_enum
  {
    UNKNOWN = 0, /**<Enum type for an unknown test_suite command. */
    LIST,        /**<Enum type for the list command. */
    EXECUTE      /**<Enum type for the execute command. */
  } test_suite_cmd;

  typedef std::vector<std::string> t_string_vector;

  typedef struct test_unit_args_struct
  {
    std::string unit;
    t_string_vector tests;
  } test_unit_args;

  typedef struct test_suite_args_struct
  {
    test_suite_args_struct()
      : cmd(UNKNOWN), initialized(false), brief(false)
    {}
    test_suite_cmd cmd;
    bool initialized;
    bool brief;
    std::vector<test_unit_args> unit_args;
  } test_suite_args;

  test_suite();

  typedef std::shared_ptr<unit_test> unit_test_ptr;
  typedef std::map<std::string, unit_test_ptr> t_unit_test_map;
  typedef t_unit_test_map::value_type value_type;

  struct unit_executer : public std::unary_function<unit_test_ptr, void>
  {
    unit_executer();
    void operator()(test_suite::value_type &x);
    
    bool succeeded;
  };

  struct unit_lister : public std::unary_function<unit_test_ptr, void>
  {
    unit_lister(std::ostream &o, bool b = false);
    void operator()(const test_suite::value_type &x) const;
    std::ostream &out;
    bool brief;
  };

public:
  struct suite_result
  {
    
  };

public:
  virtual ~test_suite();
  
  /**
   * @brief Initialize test_suite
   *
   * Reads, parses and initializes the
   * test_suite.
   *
   * @param argc Number of arguments.
   * @param argv List of arguments.
   */
  void init(int argc, char *argv[]);

  /**
   * @brief Register a new test_unit object.
   *
   * Registers a new test_unit object identified
   * by the given name.
   *
   * @param utest test_unit object.
   */
  void register_unit(unit_test *utest);

  /**
   * @brief Executes the test_suite.
   *
   * Executes all test unit classes or the
   * command given via init.
   * 
   * @return True if all tests succeeded.
   */
  bool run();

  /**
   * @brief Executes a specific test_unit.
   *
   * Executes the test_unit with the given name.
   * If there is no test_unit object with such a
   * name a message is displayed.
   *
   * @param unit The name of the test_unit object to execute.
   * @return True if all tests succeeded.
   */
  bool run(const std::string &unit);

  /**
   * @brief Executes a specific test_unit.
   *
   * Executes the test_unit with the given args.
   * Arguments contain name of test unit and
   * a list of all tests to be executed.
   * If list is empty all tests are executed.
   * If there is no test_unit object with such a
   * name a message is displayed.
   *
   * @param unit_args The arguments of the test_unit to execute.
   * @return True if all tests succeeded.
   */
  bool run(const test_unit_args &unit_args);

  /**
   * @brief Executes a single test of a test_unit.
   *
   * Executes a single test of a test_unit object,
   * identified by the name of the test and the name
   * of the test_unit object.
   * If test couldn't be found a message is displayed.
   *
   * @param unit Name of the test_unit object.
   * @param test Name of the test to execute.
   * @return True if test succeeded.
   */
  bool run(const std::string &unit, const std::string &test);

private:
  test_suite_args args_;
  t_unit_test_map unit_test_map_;
};

}

#endif /* TEST_SUITE_HPP */
