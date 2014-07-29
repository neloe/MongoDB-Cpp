/*!
 * \file basic_connection.h
 * \author Nathan Eloe
 * \brief Declaration of a basic, naive connection to the database
 */

#pragma once
#include <memory>
#include <string>
#include <zmq.hpp>

#include "../bson/document.h"
#include "../bson/element.h"

namespace mongo
{
  class BasicConnection
  {
    private:
      
      struct msg_header {int len, reqID, reTo, opCode;};
      struct reply_pre {msg_header head; int rFlags; long curID; int start, numRet;};
      const static size_t _ID_MAX_SIZE = 256;
      static int m_req_id;
      static std::shared_ptr<zmq::context_t> m_context;
      
      std::shared_ptr<zmq::socket_t>m_sock;
      char m_id[_ID_MAX_SIZE];
      size_t m_id_size;
      
      void _msg_send(std::string message);
      void _msg_recv(reply_pre & intro, std::shared_ptr<unsigned char> & docs);
    public:
      BasicConnection(zmq::context_t * ctx = nullptr);
      BasicConnection(zmq::context_t & ctx): BasicConnection(&ctx) {}
      BasicConnection(const std::string host, const std::string port = "27017", zmq::context_t * ctx = nullptr);
      BasicConnection(zmq::context_t & ctx, const std::string host, const std::string port = "27017"):BasicConnection(host, port, &ctx) {}
      ~BasicConnection();
      
      void connect(const std::string host, const std::string port = "27017");
      
      bson::Document findOne(const std::string collection, const bson::Document query = bson::Document(), 
			     const bson::Document projection = bson::Document(), const int flags = 0,
			     const int skip = 0
			    );
      
      static std::shared_ptr<zmq::context_t> get_context() {return m_context;}
      static void set_context(zmq::context_t* ctx) {m_context = std::shared_ptr<zmq::context_t>(ctx);}
      static void set_context(zmq::context_t& ctx) {m_context = std::shared_ptr<zmq::context_t>(&ctx);}
  };
}