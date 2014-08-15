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
      //Cursor ID
      long m_id;
      //The data to decode
      std::shared_ptr<unsigned char> m_docs;
      //Keep track of the position in the bytestring
      int m_lastpos, m_strsize;
      //The name of the collection
      std::string m_coll;
      //A pointer to a client so additional information can be pulled
      std::shared_ptr<MongoClient> m_client;
      //Private constructor only to be used by the MongoClient class
      Cursor(const long id, std::shared_ptr<unsigned char> docs, const int ds, const std::string& coll, MongoClient & mc);
    public:
      Cursor() = default;
      ~Cursor() {m_client -> _kill_cursor(m_id);}
      /*!
       * \brief determines whether there are any more documents to get or decode
       * \pre None
       * \post None
       * \return True if more documents to get, false otherwise
       */
      bool more () const {return m_id != 0 || m_lastpos < m_strsize; }
      /*!
       * \brief returns the next document in the resulting set of documents
       * \pre None
       * \post May make a database hit, if necessary (more documents stored on the database)
       * \return The next document in the resulting set of docs
       */
      bson::Document next();
  };
}