#include "json/json_object.hpp"
#include "json/json_string.hpp"

#include <algorithm>
#include <glob.h>

namespace oos {

json_object::json_object()
  : json_type("json_object")
{}

json_object::json_object(const json_value &x)
  : json_type("json_object")
{
  const json_object *o = x.value_type<json_object>();
  if (o) {
    string_value_map_.insert(o->begin(), o->end());
  } else {
    throw std::logic_error("json_value isn't of type json_object");
  }
}

json_object& json_object::operator=(const json_value &x)
{
  const json_object *o = x.value_type<json_object>();
  if (o) {
    string_value_map_.insert(o->begin(), o->end());
  } else {
    throw std::logic_error("json_value isn't of type json_object");
  }
  return *this;
}

json_object::~json_object()
{}

bool json_object::parse(std::istream &in)
{
  json_type::parser.parse(in);

  return true;
}

void json_object::print(std::ostream &out) const
{
  out << "{ ";

  t_string_value_map::const_iterator first = string_value_map_.begin();
  t_string_value_map::const_iterator last = string_value_map_.end();

  while (first != last) {
    out << first->first << " : " << first->second;
    if (++first != last) {
      out << ", ";
    }
  }

  out << " }";
}

void json_object::clear()
{
  string_value_map_.clear();
}

bool json_object::empty() const
{
  return string_value_map_.empty();
}

json_object::iterator json_object::begin()
{
  return string_value_map_.begin();
}

json_object::const_iterator json_object::begin() const
{
  return string_value_map_.begin();
}

json_object::iterator json_object::end()
{
  return string_value_map_.end();
}

json_object::const_iterator json_object::end() const
{
  return string_value_map_.end();
}

bool json_object::contains(const std::string &k)
{
  return string_value_map_.count(json_string(k)) > 0;
}

json_value& json_object::operator[](const std::string &key)
{
  return string_value_map_[key];
}

size_t json_object::size() const
{
  return string_value_map_.size();
}

void json_object::insert(const json_string &key, const json_value &val)
{
  string_value_map_.insert(std::make_pair(key, val));
}

std::istream& operator>>(std::istream &str, json_object &obj)
{
  obj = json_type::parser.parse(str);
  return str;
}

}
