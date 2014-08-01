/*!
 * \file mongoclient.cpp
 * \author Nathan Eloe
 * \brief Implementation of the basic, naive connection to the database
 */

#include "mongoclient.h"
#include "cursor.h"
#include <algorithm>
#include <memory>
#include <string>
#include <sstream>
#include <zmq.hpp>
#include "../bson/template_spec/convert_utils.h"
#include <iostream>

namespace mongo
{
  int MongoClient::m_req_id = 1;
  std::shared_ptr<zmq::context_t> MongoClient::m_context = nullptr;
  
  thread_local std::map<std::string, std::shared_ptr<zmq::socket_t>> MongoClient::m_socks;
  
  MongoClient::MongoClient(zmq::context_t * ctx):m_sock(nullptr)
  {
    if (m_context == nullptr)
    {
      if (ctx != nullptr)
	m_context = std::shared_ptr<zmq::context_t>(ctx);
      else
	m_context = std::make_shared<zmq::context_t>(zmq::context_t(1));
    }
    //m_sock = std::make_shared<zmq::socket_t>(zmq::socket_t(*m_context, ZMQ_STREAM));
  }
  MongoClient::MongoClient(const std::string host, const std::string port, zmq::context_t* ctx): MongoClient(ctx)
  {
    connect(host, port);
  }

  void MongoClient::connect(const std::string host, const std::string port)
  {
    std::string connstr = "tcp://" + host + ":" + port;
    if (m_socks.count(connstr) == 0)
    {
      m_socks[connstr] = std::make_shared<zmq::socket_t>(zmq::socket_t(*m_context, ZMQ_STREAM));
      m_socks[connstr]->connect(connstr.c_str());
    }
    m_sock = m_socks[connstr];
    m_id_size = _ID_MAX_SIZE;
    m_sock->getsockopt(ZMQ_IDENTITY, m_id, &m_id_size);
    return;
  }
  
  MongoClient::~MongoClient()
  {

  }
  
  void MongoClient::_msg_send(std::string message)
  {
    zmq::message_t send(message.size());
    std::memcpy((void*)send.data(), message.c_str(), message.size());
    m_sock->send(m_id, m_id_size, ZMQ_SNDMORE);
    m_sock->send(send, ZMQ_SNDMORE);
  }
  void MongoClient::_msg_recv(reply_pre & intro, std::shared_ptr<unsigned char> & docs)
  {
    zmq::message_t id, reply;
    int consumed, goal, size;
    m_sock->recv(&id);
    m_sock->recv(&reply);
    memcpy(&(intro.head), (char*)reply.data(), 16);
    memcpy(&(intro.rFlags), (char*)reply.data() + 16, 4);
    memcpy(&(intro.curID), (char*)reply.data() + 20, 8);
    memcpy(&(intro.start), (char*)reply.data() + 28, 4);
    memcpy(&(intro.numRet), (char*)reply.data() + 32, 4);
    goal = intro.head.len - 36;
    docs = std::shared_ptr<unsigned char>(new unsigned char [intro.head.len - 36], []( unsigned char *p ) { delete[] p; });
    memcpy(docs.get(), (char*)reply.data() + 36, reply.size() - 36);
    consumed = reply.size() - 36;
    while (consumed < goal)
    {
      zmq::message_t intid, intreply;
      m_sock->recv(&intid);
      m_sock->recv(&intreply);
      memcpy(docs.get() + consumed, (char*)intreply.data(), std::min(static_cast<int>(intreply.size()), goal - consumed));
      consumed += std::min(static_cast<int>(intreply.size()), goal - consumed);
    }
    return;
  }
  void MongoClient::_kill_cursor(const long cursorID)
  {
    std::ostringstream kill;
    _encode_header(kill, 16, KILL_CURSORS);
    bson::Element::encode(kill, 0);
    bson::Element::encode(kill, 1);
    bson::Element::encode(kill, cursorID);
    _msg_send(kill.str());
  }
  
  void MongoClient::_encode_header(std::ostringstream&ss, const int size, const int type)
  {
    bson::Element::encode(ss, size + 16);
    bson::Element::encode(ss, m_req_id++);
    bson::Element::encode(ss, 0);
    bson::Element::encode(ss, type);
  }
  
  void MongoClient::_more_cursor(Cursor& c)
  {
    reply_pre intro;
    std::ostringstream ss;
    _encode_header(ss, 17 + c.m_coll.size(), GET_MORE);
    bson::Element::encode(ss, 0);
    ss << c.m_coll.c_str() << bson::X00;
    bson::Element::encode(ss, 0);
    bson::Element::encode(ss, c.m_id);
    _msg_send(ss.str());
    _msg_recv(intro, c.m_docs);
    c.m_id = intro.curID;
    c.m_strsize = intro.head.len - 36;
    c.m_lastpos = 0;
    return;
  }

  
  bson::Document MongoClient::findOne(const std::string collection, const bson::Document query, 
					  const bson::Document projection, const int flags, const int skip)
  {
    std::ostringstream querystream, header;
    bson::Document qd;
    zmq::message_t reply;
    reply_pre intro;
    int num_returned;
    int docsize, headsize;
    bson::Document result;
    std::shared_ptr<unsigned char> data;
    
    qd.add("$query", query);
    bson::Element::encode(querystream, flags);
    querystream << collection.c_str() << bson::X00;
    bson::Element::encode(querystream, skip);
    bson::Element::encode(querystream, 1);
    bson::Element::encode(querystream, qd);
    if (projection.field_names().size() > 0)
      bson::Element::encode(querystream, projection);
    _encode_header(header, static_cast<int>(querystream.tellp()), QUERY);
    _msg_send(header.str() + querystream.str());
    _msg_recv(intro, data);
    if (intro.numRet == 1)
    {
      bson::Element e;
      e.decode(data.get(), bson::DOCUMENT);
      result = e.data<bson::Document>();
    }
    return result;
    
  }
  
  Cursor MongoClient::find(const std::string collection, const bson::Document query, 
				   const bson::Document projection, const int flags, const int skip)
  {
    std::ostringstream querystream, header;
    bson::Document qd;
    zmq::message_t reply;
    reply_pre intro;
    int num_returned;
    int docsize, headsize;
    bson::Document result;
    std::shared_ptr<unsigned char> data;
    
    qd.add("$query", query);
    bson::Element::encode(querystream, flags);
    querystream << collection.c_str() << bson::X00;
    bson::Element::encode(querystream, skip);
    bson::Element::encode(querystream, 0);
    bson::Element::encode(querystream, qd);
    if (projection.field_names().size() > 0)
      bson::Element::encode(querystream, projection);
    _encode_header(header, static_cast<int>(querystream.tellp()), QUERY);
    _msg_send(header.str() + querystream.str());
    _msg_recv(intro, data);
    std::cout << "docs in reply: " << intro.numRet << std::endl;
    return Cursor(intro.curID, data, intro.head.len - 36, collection, this);
  }
  
  void MongoClient::update(const std::string collection, const bson::Document selector, const bson::Document update,
			   const bool upsert, const bool multi)
  {
    std::ostringstream msg, header;
    bson::Element::encode(msg, 0);
    msg << collection.c_str() << bson::X00;
    bson::Element::encode(msg, static_cast<int>(upsert) | (static_cast<int>(multi)<<1));
    bson::Element::encode(msg, selector);
    bson::Element::encode(msg, update);
    _encode_header(header, static_cast<int>(msg.tellp()), UPDATE);
    _msg_send(header.str() + msg.str());
    return;
  }
  
  void MongoClient::insert(const std::string collection, const bson::Document toinsert)
  {
    std::ostringstream msg, header;
    bson::Element::encode(msg, 0);
    msg << collection.c_str() << bson::X00;
    bson::Element::encode(msg, toinsert);
    _encode_header(header, static_cast<int>(msg.tellp()), INSERT);
    _msg_send(header.str() + msg.str());
    return;
  }
  void MongoClient::remove(const std::string collection, const bson::Document selector, const bool rm_one)
  {
    std::ostringstream msg, header;
    bson::Element::encode(msg, 0);
    msg << collection.c_str() << bson::X00;
    bson::Element::encode(msg, selector);
    bson::Element::encode(msg, static_cast<int>(rm_one));
    _encode_header(header, static_cast<int>(msg.tellp()), DELETE);
    _msg_send(header.str() + msg.str());
    return;
  }
  
  bson::Document MongoClient::runCommand(const std::string dbname, const bson::Document cmd)
  {
    return findOne(dbname + ".$cmd", cmd);
  }
}