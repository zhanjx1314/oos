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

#include "database/transaction.hpp"
#include "database/transaction_helper.hpp"
#include "database/session.hpp"
#include "database/database.hpp"
#include "database/database_exception.hpp"

#include "object/object.hpp"

#include <sstream>
#include <memory>

using namespace std;

namespace oos {

void transaction::on_insert(object_proxy *proxy)
{
  /*****************
   * 
   * backup inserted object
   * on rollback the object
   * is removed from object
   * store
   * 
   *****************/
  id_iterator_map_t::iterator i = id_map_.find(proxy->obj->id());
  if (i == id_map_.end()) {
    // create insert action and insert object
    action_inserter ai(action_list_);
    iterator j = ai.insert(proxy);
    if (j == action_list_.end()) {
      // should not happen
    } else {
      id_map_.insert(std::make_pair(proxy->obj->id(), j));
    }
  } else {
    // ERROR: an object with that id already exists
    std::stringstream msg;
    msg << "an object with id " << proxy->obj->id() << " already exists";
    throw database_exception("database", msg.str().c_str());
  }
}

void transaction::on_update(object_proxy *proxy)
{
  /*****************
   * 
   * backup updated object
   * on rollback the object
   * is restored to old values
   * 
   *****************/
  if (id_map_.find(proxy->obj->id()) == id_map_.end()) {
    backup(new update_action(proxy), proxy->obj);
  } else {
    // An object with that id already exists
    // do nothing because the object is already
    // backed up
  }
}

void transaction::on_delete(object_proxy *proxy)
{
  /*****************
   * 
   * backup deleted object
   * on rollback the object
   * is restored into the
   * object store
   * 
   *****************/

  id_iterator_map_t::iterator i = id_map_.find(proxy->obj->id());
  if (i == id_map_.end()) {
    backup(new delete_action(proxy->node->type.c_str(), proxy->obj->id()), proxy->obj);
  } else {
    action_remover ar(action_list_);
    ar.remove(i->second, proxy);
  }
}

transaction::transaction(session &db)
  : db_(db)
  , id_(0)
{}

transaction::~transaction()
{
}

long transaction::id_counter = 0;

long
transaction::id() const
{
  return id_;
}

void
transaction::begin()
{
  /**************
   * 
   * On begin transaction gets unique
   * id. Transaction is pushed to dbs
   * transactions stack. Previous transactions
   * observer is beeing unregistered and
   * this transactions observer is regsitered.
   *
   **************/
  id_ = ++transaction::id_counter;
  db_.begin(*this);
}

void
transaction::commit()
{
  if (!db_.current_transaction() || db_.current_transaction() != this) {
    throw database_exception("transaction", "transaction isn't current transaction");
  } else {
    // commit all transaction actions
    db_.commit(*this);
    // clear actions
    cleanup();
  }
}

void
transaction::rollback()
{
  if (!db_.current_transaction() || db_.current_transaction() != this) {
    throw database_exception("transaction", "transaction isn't current transaction");
  } else {
    /**************
     *
     * rollback transaction
     * restore objects
     * and finally pop transaction
     * clear insert action map
     *
     **************/

    while (!action_list_.empty()) {
      iterator i = action_list_.begin();
      std::unique_ptr<action> a(*i);
      action_list_.erase(i);
      restore(a.get());
    }

    db_.rollback();

    // clear container
    cleanup();
  }
}

session&
transaction::db()
{
  return db_;
}

const session&
transaction::db() const
{
  return db_;
}

void
transaction::backup(action *a, const object *o)
{
  /*************
   * 
   * try to find object with
   * id in action map
   * 
   *************/
  backup_visitor bv;
  bv.backup(a, o, &object_buffer_);
  iterator i = action_list_.insert(action_list_.end(), a);
  id_map_.insert(std::make_pair(o->id(), i));
}

void transaction::restore(action *a)
{
  restore_visitor rv;
  rv.restore(a, &object_buffer_, &db_.ostore());
}

void transaction::cleanup()
{
  while (!action_list_.empty()) {
    action *a = action_list_.front();
    delete a;
    action_list_.pop_front();
  }

  object_buffer_.clear();
  id_map_.clear();
  db_.pop_transaction();
}

}
