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
 * \file cursor.cpp
 * \author Nathan Eloe
 * \brief Implementation of cursor class
 */

#include "cursor.h"
#include <iostream>

namespace mongo
{
  Cursor::Cursor(const long id, std::shared_ptr<unsigned char> docs, const int ds, const std::string& coll, MongoClient & mc):
    m_id(id), m_docs(docs), m_lastpos(0), m_strsize(ds), m_client(std::make_shared<MongoClient>(mc)), m_coll(coll) {}
  
  bson::Document Cursor::next()
  {
    bson::Element e;
    m_lastpos +=e.decode(m_docs.get() + m_lastpos, bson::DOCUMENT);
    if (m_lastpos >= m_strsize && m_id != 0)
    {
      (*m_client)._more_cursor(*this);
    }
    return e.data<bson::Document>();
  }
}