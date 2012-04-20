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

#ifndef OBJECT_SERIALIZER_HPP
#define OBJECT_SERIALIZER_HPP

#ifdef WIN32
  #ifdef oos_EXPORTS
    #define OOS_API __declspec(dllexport)
    #define EXPIMP_TEMPLATE
  #else
    #define OOS_API __declspec(dllimport)
    #define EXPIMP_TEMPLATE extern
  #endif
#else
  #define OOS_API
#endif

#include "object/object_atomizer.hpp"

namespace oos {

class object;
class object_store;
class byte_buffer;
class varchar_base;

/**
 * @cond OOS_DEV
 * @class object_serializer
 * @brief serialize objects to a given buffer.
 *
 * The object_serializer class serialize a given
 * object to a given buffer or deserialize a given
 * object from a given buffer.
 * On deserialization the class doesn't take care
 * of the correctness of the object and the current
 * memory the buffer points to.
 * The application is responsible for this correctness.
 */
class OOS_API object_serializer : public object_atomizer
{
public:
  /**
   * Creates an object_serializer
   */
  object_serializer();

  virtual ~object_serializer();

  /**
   * Serialize the given object to the given buffer
   *
   * @param o The object to serialize.
   * @param buffer The byte_buffer to serialize to.
   * @return True on success.
   */
  bool serialize(const object *o, byte_buffer &buffer);

  /**
   * Serialize the given object to the given buffer
   *
   * @param o The object to deserialize.
   * @param buffer The byte_buffer to deserialize from.
   * @param ostore The object_store where the object resides.
   * @return True on success.
   */
  bool deserialize(object *o, byte_buffer &buffer, object_store *ostore);

private:
	virtual void write(const char* id, char c);
	virtual void write(const char* id, float f);
	virtual void write(const char* id, double f);
	virtual void write(const char* id, short x);
	virtual void write(const char* id, int i);
	virtual void write(const char* id, long l);
	virtual void write(const char* id, unsigned short x);
	virtual void write(const char* id, unsigned int x);
	virtual void write(const char* id, unsigned long x);
	virtual void write(const char* id, bool b);
	virtual void write(const char* id, const char *c);
	virtual void write(const char* id, const std::string &s);
  virtual void write(const char*, const varchar_base &s);
	virtual void write(const char* id, const object_base_ptr &x);
	virtual void write(const char* id, const object_container &x);

	virtual void read(const char* id, char &c);
	virtual void read(const char* id, float &f);
	virtual void read(const char* id, double &f);
	virtual void read(const char* id, short &i);
	virtual void read(const char* id, int &i);
	virtual void read(const char* id, long &l);
	virtual void read(const char* id, unsigned short &x);
	virtual void read(const char* id, unsigned int &x);
	virtual void read(const char* id, unsigned long &x);
	virtual void read(const char* id, bool &b);
	virtual void read(const char* id, char *&c);
	virtual void read(const char* id, std::string &s);
  virtual void read(const char*, varchar_base &s);
  virtual void read(const char* id, object_base_ptr &x);
	virtual void read(const char* id, object_container &x);
  
  void write_object_list_item(const object *o);
  void write_object_container_item(const object *o);
  void write_object_vector_item(const object *o, unsigned int &index);

private:
  object_store *ostore_;
  byte_buffer *buffer_;
};
/// @endcond
}

#endif /* OBJECT_SERIALIZER_HPP */
