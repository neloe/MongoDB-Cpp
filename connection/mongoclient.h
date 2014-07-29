/*!
 * \file mongoclient.h
 * \author Nathan Eloe
 * \brief A mongo client that shares from a connection pool
 */

#pragma once

#include "basic_connection.h"
#include <map>
#include <memory>
#include <queue>
#include <string>
#include <zmq.hpp>

namespace mongo
{
  class MongoClient
  {
    private:
      static std::map<std::string, std::queue<std::shared_ptr<BasicConnection>>> m_conn_pool;
      static std::shared_ptr<zmq::context_t> m_context;
      std::string m_connstr;
      std::shared_ptr<BasicConnection> m_conn;
      
    public:
      MongoClient(zmq::context_t * ctx, const std::string & host = "localhost", const std::string & port = "27017");
      MongoClient(zmq::context_t & ctx, const std::string & host = "localhost", const std::string & port = "27017"): MongoClient(&ctx, host, port) {}
      MongoClient(const std::string & host = "localhost", const std::string & port = "27017"): MongoClient(nullptr, host, port) {}
      ~MongoClient() {m_conn_pool[m_connstr].push(m_conn);}
      
      bson::Document findOne(const std::string collection, const bson::Document query = bson::Document(), 
			     const bson::Document projection = bson::Document(), const int flags = 0,
			     const int skip = 0) {return m_conn -> findOne(collection, query, projection, flags, skip);}
  };
}