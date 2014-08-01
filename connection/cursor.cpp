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
    //std::cout << "in next()" << std::endl;
    if (m_lastpos >= m_strsize && m_id != 0)
    {
      (*m_client)._more_cursor(*this);
    }
    return e.data<bson::Document>();
  }
}