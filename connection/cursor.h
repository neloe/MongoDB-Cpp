/*!
 * \file cursor.h
 * \author Nathan Eloe
 * \brief Definition of a cursor class for iterating through multiple results
 */
#pragma once
#include "mongoclient.h"
#include "../bson/document.h"
#include <memory>

namespace mongo
{
  class Cursor
  {
    private:
      
      friend class MongoClient;
      
      long m_id;
      std::shared_ptr<unsigned char> m_docs;
      int m_lastpos, m_strsize;
      std::string m_coll;
      // DO NOT TOUCH THIS PLEASE
      MongoClient * m_client;
      Cursor(const long id, std::shared_ptr<unsigned char> docs, const int ds, const std::string& coll, MongoClient* mc);
    public:
      Cursor() = default;
      ~Cursor() {m_client -> _kill_cursor(m_id);}
      bool more () const {return m_id != 0 || m_lastpos < m_strsize; }
      bson::Document next();
  };
}