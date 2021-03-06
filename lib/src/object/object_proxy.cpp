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

#include "object/object_proxy.hpp"
#include "object/object.hpp"
#include "object/object_store.hpp"

using namespace std;

namespace oos {

object_proxy::object_proxy(object_store *os)
  : prev(0)
  , next(0)
  , obj(0)
  , oid(0)
  , ref_count(0)
  , ptr_count(0)
  , ostore(os)
  , node(0)
{}


object_proxy::object_proxy(unsigned long i, object_store *os)
  : prev(0)
  , next(0)
  , obj(0)
  , oid(i)
  , ref_count(0)
  , ptr_count(0)
  , ostore(os)
  , node(0)
{}

object_proxy::object_proxy(object *o, object_store *os)
  : prev(0)
  , next(0)
  , obj(o)
  , oid((o ? o->id() : 0))
  , ref_count(0)
  , ptr_count(0)
  , ostore(os)
  , node(0)
{}

object_proxy::~object_proxy()
{
  if (ostore && id() > 0) {
    ostore->delete_proxy(id());
  }
  if (obj) {
    delete obj;
  }
  ostore = 0;
  for (ptr_set_t::iterator i = ptr_set_.begin(); i != ptr_set_.end(); ++i) {
    (*i)->proxy_ = 0;
  }
}

void object_proxy::link(object_proxy *successor)
{
  // link object proxy before this node
  prev = successor->prev;
  next = successor;
  if (successor->prev) {
    successor->prev->next = this;
  }
  successor->prev = this;
}

void object_proxy::unlink()
{
  if (prev) {
    prev->next = next;
  }
  if (next) {
    next->prev = prev;
  }
  prev = 0;
  next = 0;
  node = 0;
}

void object_proxy::link_ref()
{
  if (obj) {
    ++ref_count;
  }
}

void object_proxy::unlink_ref()
{
  if (obj) {
    --ref_count;
  }
}

void object_proxy::link_ptr()
{
  if (obj) {
    ++ptr_count;
  }
}

void object_proxy::unlink_ptr()
{
  if (obj) {
    --ptr_count;
  }
}

bool object_proxy::linked() const
{
  return node != 0;
}

void object_proxy::reset(object *o)
{
  ref_count = 0;
  ptr_count = 0;
  obj = o;
  oid = o ? o->id() : 0;
  node = 0;
}

void object_proxy::add(object_base_ptr *ptr)
{
  ptr_set_.insert(ptr);
}

bool object_proxy::remove(object_base_ptr *ptr)
{
  return ptr_set_.erase(ptr) == 1;
}

bool object_proxy::valid() const
{
  return ostore && node && prev && next;
}

unsigned long object_proxy::id() const {
  return (obj ? obj->id() : oid);
}

void object_proxy::id(unsigned long i)
{
  if (obj) {
    obj->id(i);
  } else {
    oid = i;
  }
}

std::ostream& operator <<(std::ostream &os, const object_proxy &op)
{
  os << "proxy [" << &op << "] prev [" << op.prev << "] next [" << op.next << "] object [" << op.obj << "] refs [" << op.ref_count << "] ptrs [" << op.ptr_count << "]";
  return os;
}

}
