/*!
 * \file mongoclient.cpp
 * \author Nathan Eloe
 * \brief A mongo client that shares a connection pool
 */

#include "mongoclient.h"
#include <iostream>

namespace mongo
{
  std::map<std::string, std::queue<std::shared_ptr<BasicConnection>>> MongoClient::m_conn_pool;
  std::shared_ptr<zmq::context_t> MongoClient::m_context = nullptr;
  
  MongoClient::MongoClient(zmq::context_t* ctx, const std::string& host, const std::string& port): m_connstr("tcp://" + host + ":" + port)
  {
    if (m_context == nullptr)
    {
      if (ctx)
	m_context = std::shared_ptr<zmq::context_t>(ctx);
      else
	m_context = std::make_shared<zmq::context_t>(zmq::context_t(1));
    }
    if (!m_conn_pool[m_connstr].empty())
    {
      m_conn = m_conn_pool[m_connstr].front();
      m_conn_pool[m_connstr].pop();
    }
    else
      m_conn = std::make_shared<BasicConnection>(BasicConnection(host, port));
  }


}