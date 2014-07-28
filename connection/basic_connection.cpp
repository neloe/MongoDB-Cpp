/*!
 * \file basic_connection.cpp
 * \author Nathan Eloe
 * \brief Implementation of the basic, naive connection to the database
 */

#include "basic_connection.h"
#include <memory>
#include <string>
#include <sstream>
#include <zmq.hpp>
#include "../bson/template_spec/convert_utils.h"

namespace mongo
{
  int BasicConnection::m_req_id = 1;
  std::shared_ptr<zmq::context_t> BasicConnection::m_context = nullptr;
  
  BasicConnection::BasicConnection(zmq::context_t * ctx)
  {
    if (m_context == nullptr)
    {
      if (ctx != nullptr)
	m_context = std::shared_ptr<zmq::context_t>(ctx);
      else
	m_context = std::make_shared<zmq::context_t>(zmq::context_t(1));
    }
    m_sock = std::make_shared<zmq::socket_t>(zmq::socket_t(*m_context, ZMQ_STREAM));
  }
  BasicConnection::BasicConnection(const std::string host, const std::string port, zmq::context_t* ctx): BasicConnection(ctx)
  {
    connect(host, port);
  }

  void BasicConnection::connect(const std::string host, const std::string port)
  {
    std::string connstr = "tcp://" + host + ":" + port;
    m_sock->connect(connstr.c_str());
    m_id_size = _ID_MAX_SIZE;
    m_sock->getsockopt(ZMQ_IDENTITY, m_id, &m_id_size);
    return;
  }
  
  void BasicConnection::_msg_send(std::string message)
  {
    zmq::message_t send(message.size());
    std::memcpy((void*)send.data(), message.c_str(), message.size());
    m_sock->send(m_id, m_id_size, ZMQ_SNDMORE);
    m_sock->send(send, ZMQ_SNDMORE);
  }
  
  BasicConnection::~BasicConnection()
  {

  }

  bson::Document BasicConnection::findOne(const std::string collection, const bson::Document query, 
					  const bson::Document projection, const int flags, const int skip)
  {
    std::ostringstream querystream, header;
    bson::Document qd;
    zmq::message_t reply;
    int num_returned;
    bson::Document result;
    
    qd.add("$query", query);
    bson::Element::encode(querystream, flags);
    querystream << collection.c_str() << bson::X00;
    bson::Element::encode(querystream, skip);
    bson::Element::encode(querystream, 1);
    bson::Element::encode(querystream, qd);
    if (projection.field_names().size() > 0)
      bson::Element::encode(querystream, projection);
    bson::Element::encode(header, static_cast<int>(querystream.tellp()) + 16);
    bson::Element::encode(header, m_req_id++);
    bson::Element::encode(header, 0);
    bson::Element::encode(header, 2004);
    _msg_send(header.str() + querystream.str());
    
    //ignore this, I guess.  I don't know enough to know better
    m_sock->recv(&reply);
    m_sock->recv(&reply);
    memcpy(&num_returned, (char*)reply.data() + 32, 4);
    if (num_returned == 1)
    {
      bson::Element e;
      e.decode((unsigned char*)reply.data() + 36, bson::DOCUMENT);
      result = e.data<bson::Document>();
    }
    return result;
    
  }
  
}