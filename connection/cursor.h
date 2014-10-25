/*
  Copyright (c) Nathan Eloe, 2014
  This file is part of MongoDB-Cpp.

  MongoDB-Cpp is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  MongoDB-Cpp is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with MongoDB-Cpp.  If not, see <http://www.gnu.org/licenses/>.
*/
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