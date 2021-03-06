#include "TransactionTestUnit.hpp"

#include "../Item.hpp"

#include "object/object_view.hpp"

#include "database/session.hpp"
#include "database/database_exception.hpp"

using namespace oos;


TransactionTestUnit::TransactionTestUnit(const std::string &name, const std::string &msg, std::string const &db)
  : unit_test(name, msg)
  , db_(db)
  , session_(nullptr)
{
  add_test("simple", std::bind(&TransactionTestUnit::test_simple, this), "simple database test");
  add_test("complex", std::bind(&TransactionTestUnit::test_with_sub, this), "object with sub object database test");
  add_test("list", std::bind(&TransactionTestUnit::test_with_list, this), "object with object list database test");
  add_test("vector", std::bind(&TransactionTestUnit::test_with_vector, this), "object with object vector database test");
}


TransactionTestUnit::~TransactionTestUnit() {}

void
TransactionTestUnit::initialize()
{
  ostore_.insert_prototype<Item>("item");
  ostore_.insert_prototype<ObjectItem<Item>, Item>("object_item");
  ostore_.insert_prototype<ItemPtrList>("item_ptr_list");
  ostore_.insert_prototype<ItemPtrVector>("item_ptr_vector");
//  ostore_.insert_prototype<playlist>("playlist");
  ostore_.insert_prototype<album>("album");
  ostore_.insert_prototype<track>("track");

  // create session
  session_ = create_session();
}

void
TransactionTestUnit::finalize()
{
  delete session_;
  ostore_.clear(true);
}

void
TransactionTestUnit::test_simple()
{
  // open connection
  session_->open();
  // create schema
  session_->create();

  // create new transaction    
  transaction tr(*session_);
  try {
    // begin transaction
    tr.begin();

    // ... do some object modifications
    typedef object_ptr<Item> item_ptr;
    typedef object_view<Item> item_view;
    // insert new object
    item_ptr item = ostore_.insert(new Item("Hello World", 70));
    UNIT_ASSERT_GREATER(item->id(), 0UL, "item has invalid object id");
    tr.commit();

    tr.begin();
    // modify object
    item->set_int(120);
    UNIT_ASSERT_EQUAL(item->get_int(), 120, "item has invalid int value");

    transaction tr2(*session_);
    try {
      // begin inner transaction
      tr2.begin();
      // change name again
      item->set_int(170);

      UNIT_ASSERT_EQUAL(item->get_int(), 170, "item has invalid int value");
      // rollback transaction
      tr2.rollback();

      UNIT_ASSERT_EQUAL(item->get_int(), 120, "item has invalid int value");
    } catch (std::exception &ex) {
      UNIT_WARN("transaction [" << tr2.id() << "] rolled back: " << ex.what());
      tr2.rollback();
    }
    tr.rollback();

    UNIT_ASSERT_EQUAL(item->get_int(), 70, "item has invalid int value");

    tr.begin();
    // delete object
    UNIT_ASSERT_TRUE(ostore_.is_removable(item), "couldn't delete item");

    ostore_.remove(item);

    tr.rollback();

    item_view view(ostore_);

    UNIT_ASSERT_FALSE(view.empty(), "item view is empty");
    UNIT_ASSERT_EQUAL((int)view.size(), 1, "more than one item in view");

    item = view.front();

    UNIT_ASSERT_EQUAL(item->get_string(), "Hello World", "invalid item name");
    UNIT_ASSERT_EQUAL(item->get_int(), 70, "invalid item int value");

    tr.begin();

    UNIT_ASSERT_TRUE(ostore_.is_removable(item), "couldn't delete item");

    ostore_.remove(item);

    tr.commit();

    UNIT_ASSERT_TRUE(view.empty(), "item view is empty");
  } catch (database_exception &ex) {
    // error, abort transaction
    UNIT_WARN("transaction [" << tr.id() << "] rolled back: " << ex.what());
    tr.rollback();
  }
  session_->drop();
  // close db
  session_->close();
}

void
TransactionTestUnit::test_with_sub()
{
  // open connection
  session_->open();

  // load data
  session_->create();

  // load data
  /****************
  *
  * comment this statement and the following
  * will happen if data exists:
  * data won't be load
  * a car object with id 1 is created
  * and an exception on insert is thrown
  * because there is already a car object
  * with id 1
  ****************/
//  session_->load();

  // create new transaction    
  transaction tr(*session_);
  try {
    // begin transaction
    tr.begin();
    // ... do some object modifications
    typedef ObjectItem<Item> object_item_t;
    typedef object_ptr<object_item_t> object_item_ptr;
    typedef object_ptr<Item> item_ptr;
    // insert new object
    object_item_ptr object_item = ostore_.insert(new object_item_t("Foo", 42));

    UNIT_ASSERT_GREATER(object_item->id(), 0UL, "invalid object item");

    item_ptr item = object_item->ptr();

    UNIT_ASSERT_GREATER(item->id(), 0UL, "invalid item");

    item->set_int(120);
    item->set_string("Bar");

    UNIT_ASSERT_EQUAL(item->get_int(), 120, "invalid item int value");
    UNIT_ASSERT_EQUAL(item->get_string(), "Bar", "invalid item string value");

    tr.commit();

    UNIT_ASSERT_EQUAL(item->get_int(), 120, "invalid item int value");
    UNIT_ASSERT_EQUAL(item->get_string(), "Bar", "invalid item string value");

    tr.begin();

    object_view<object_item_t> oview(ostore_);

    UNIT_ASSERT_FALSE(oview.empty(), "object item view couldn't be empty");

    UNIT_ASSERT_TRUE(ostore_.is_removable(object_item), "couldn't remove object item");

    ostore_.remove(object_item);

    UNIT_ASSERT_TRUE(oview.empty(), "object item view must be empty");

    tr.rollback();

    UNIT_ASSERT_FALSE(oview.empty(), "object item view couldn't be empty");

  } catch (database_exception &ex) {
    // error, abort transaction
    UNIT_WARN("caught database exception: " << ex.what() << " (start rollback)");
    tr.rollback();
  } catch (object_exception &ex) {
    // error, abort transaction
    UNIT_WARN("caught object exception: " << ex.what() << " (start rollback)");
    tr.rollback();
  }
  session_->drop();
  // close db
  session_->close();
}

void
TransactionTestUnit::test_with_list()
{
  typedef object_ptr<ItemPtrList> itemlist_ptr;
  typedef ItemPtrList::value_type item_ptr;

  // open connection
  session_->open();

  // load data
  session_->create();

  // create new transaction    
  transaction tr(*session_);
  try {
    // begin transaction
    tr.begin();
    // ... do some object modifications

    itemlist_ptr itemlist = ostore_.insert(new ItemPtrList);

    UNIT_ASSERT_GREATER(itemlist->id(), 0UL, "invalid item list");
    UNIT_ASSERT_TRUE(itemlist->empty(), "item list must be empty");

    tr.commit();

    tr.begin();
    for (int i = 0; i < 2; ++i) {
      std::stringstream name;
      name << "Item " << i+1;
      item_ptr item = ostore_.insert(new Item(name.str()));

      UNIT_ASSERT_GREATER(item->id(), 0UL, "invalid item");

      itemlist->push_back(item);
    }

    UNIT_ASSERT_FALSE(itemlist->empty(), "item list couldn't be empty");
    UNIT_ASSERT_EQUAL((int)itemlist->size(), 2, "invalid item list size");

    tr.rollback();

    UNIT_ASSERT_TRUE(itemlist->empty(), "item list must be empty");
    UNIT_ASSERT_EQUAL((int)itemlist->size(), 0, "invalid item list size");

    tr.begin();

    for (int i = 0; i < 2; ++i) {
      std::stringstream name;
      name << "Item " << i+1;
      item_ptr item = ostore_.insert(new Item(name.str()));

      UNIT_ASSERT_GREATER(item->id(), 0UL, "invalid item");

      itemlist->push_back(item);
    }

    UNIT_ASSERT_FALSE(itemlist->empty(), "item list couldn't be empty");
    UNIT_ASSERT_EQUAL((int)itemlist->size(), 2, "invalid item list size");

    tr.commit();

    UNIT_ASSERT_FALSE(itemlist->empty(), "item list couldn't be empty");
    UNIT_ASSERT_EQUAL((int)itemlist->size(), 2, "invalid item list size");
    tr.begin();

    itemlist->clear();

    UNIT_ASSERT_TRUE(itemlist->empty(), "item list must be empty");

    tr.rollback();

    UNIT_ASSERT_FALSE(itemlist->empty(), "item list couldn't be empty");
    UNIT_ASSERT_EQUAL((int)itemlist->size(), 2, "invalid item list size");

  } catch (database_exception &ex) {
    // error, abort transaction
    UNIT_WARN("caught database exception: " << ex.what() << " (start rollback)");
    tr.rollback();
  } catch (object_exception &ex) {
    // error, abort transaction
    UNIT_WARN("caught object exception: " << ex.what() << " (start rollback)");
    tr.rollback();
  }
  session_->drop();
  // close db
  session_->close();
}

void
TransactionTestUnit::test_with_vector()
{
  typedef object_ptr<ItemPtrVector> itemvector_ptr;
  typedef ItemPtrVector::value_type item_ptr;

  // open connection
  session_->open();

  // load data
  session_->create();

  // create new transaction    
  transaction tr(*session_);
  try {
    // begin transaction
    tr.begin();
    // ... do some object modifications

    itemvector_ptr itemvector = ostore_.insert(new ItemPtrVector);

    UNIT_ASSERT_GREATER(itemvector->id(), 0UL, "invalid item list");
    UNIT_ASSERT_TRUE(itemvector->empty(), "item list must be empty");

    tr.commit();

    tr.begin();
    for (int i = 0; i < 2; ++i) {
      std::stringstream name;
      name << "Item " << i+1;
      item_ptr item = ostore_.insert(new Item(name.str()));

      UNIT_ASSERT_GREATER(item->id(), 0UL, "invalid item");

      itemvector->push_back(item);
    }

    UNIT_ASSERT_FALSE(itemvector->empty(), "item list couldn't be empty");
    UNIT_ASSERT_EQUAL((int)itemvector->size(), 2, "invalid item list size");

    tr.rollback();

    UNIT_ASSERT_TRUE(itemvector->empty(), "item list must be empty");
    UNIT_ASSERT_EQUAL((int)itemvector->size(), 0, "invalid item list size");

    tr.begin();

    for (int i = 0; i < 2; ++i) {
      std::stringstream name;
      name << "Item " << i+1;
      item_ptr item = ostore_.insert(new Item(name.str()));

      UNIT_ASSERT_GREATER(item->id(), 0UL, "invalid item");

      itemvector->push_back(item);
    }

    UNIT_ASSERT_FALSE(itemvector->empty(), "item list couldn't be empty");
    UNIT_ASSERT_EQUAL((int)itemvector->size(), 2, "invalid item list size");

    tr.commit();

    UNIT_ASSERT_FALSE(itemvector->empty(), "item list couldn't be empty");
    UNIT_ASSERT_EQUAL((int)itemvector->size(), 2, "invalid item list size");
    tr.begin();

    itemvector->clear();

    UNIT_ASSERT_TRUE(itemvector->empty(), "item list must be empty");

    tr.rollback();

    UNIT_ASSERT_FALSE(itemvector->empty(), "item list couldn't be empty");
    UNIT_ASSERT_EQUAL((int)itemvector->size(), 2, "invalid item list size");

  } catch (database_exception &ex) {
    // error, abort transaction
    UNIT_WARN("caught database exception: " << ex.what() << " (start rollback)");
    tr.rollback();
  } catch (object_exception &ex) {
    // error, abort transaction
    UNIT_WARN("caught object exception: " << ex.what() << " (start rollback)");
    tr.rollback();
  }
  session_->drop();
  // close db
  session_->close();
}

session* TransactionTestUnit::create_session()
{
  return new session(ostore_, db_);
}

oos::object_store& TransactionTestUnit::ostore()
{
  return ostore_;
}

const oos::object_store& TransactionTestUnit::ostore() const
{
  return ostore_;
}
