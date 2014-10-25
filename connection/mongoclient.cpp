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
  MongoClient::MongoClient(const std::string & host, const std::string & port, zmq::context_t* ctx): MongoClient(ctx)
  {
    connect(host, port);
  }

  void MongoClient::connect(const std::string & host, const std::string & port)
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
    memcpy(&(intro.head), (char*)reply.data(), HEAD_SIZE);
    memcpy(&(intro.rFlags), (char*)reply.data() + HEAD_SIZE, sizeof(int));
    memcpy(&(intro.curID), (char*)reply.data() + HEAD_SIZE + sizeof(int), sizeof(long));
    memcpy(&(intro.start), (char*)reply.data() + HEAD_SIZE + sizeof(int) + sizeof(long), sizeof(int));
    memcpy(&(intro.numRet), (char*)reply.data() + HEAD_SIZE + sizeof(int) + sizeof(long)  + sizeof(int), sizeof(int));
    goal = intro.head.len - REPLYPRE_SIZE;
    docs = std::shared_ptr<unsigned char>(new unsigned char [intro.head.len - REPLYPRE_SIZE], []( unsigned char *p ) { delete[] p; });
    memcpy(docs.get(), (char*)reply.data() + REPLYPRE_SIZE, reply.size() - REPLYPRE_SIZE);
    consumed = reply.size() - REPLYPRE_SIZE;
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
    _encode_header(kill, HEAD_SIZE, KILL_CURSORS);
    bson::Element::encode(kill, 0);
    bson::Element::encode(kill, 1);
    bson::Element::encode(kill, cursorID);
    _msg_send(kill.str());
  }
  
  void MongoClient::_encode_header(std::ostringstream&ss, const int size, const int type)
  {
    bson::Element::encode(ss, size + HEAD_SIZE);
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
    c.m_strsize = intro.head.len - REPLYPRE_SIZE;
    c.m_lastpos = 0;
    return;
  }

  
  bson::Document MongoClient::findOne(const std::string & collection, const bson::Document & query, 
					  const bson::Document & projection, const int flags, const int skip)
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
    if (intro.curID != 0)
      _kill_cursor(intro.curID);
    return result;
    
  }
  
  Cursor MongoClient::find(const std::string & collection, const bson::Document & query, 
				   const bson::Document & projection, const int flags, const int skip)
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
    return Cursor(intro.curID, data, intro.head.len - REPLYPRE_SIZE, collection, *this);
  }
  
  void MongoClient::update(const std::string & collection, const bson::Document & selector, const bson::Document & update,
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
  
  void MongoClient::insert(const std::string & collection, const bson::Document & toinsert)
  {
    std::ostringstream msg, header;
    bson::Element::encode(msg, 0);
    msg << collection.c_str() << bson::X00;
    bson::Element::encode(msg, toinsert);
    _encode_header(header, static_cast<int>(msg.tellp()), INSERT);
    _msg_send(header.str() + msg.str());
    return;
  }
  void MongoClient::remove(const std::string & collection, const bson::Document & selector, const bool rm_one)
  {
    std::ostringstream msg, header;
    bson::Element::encode(msg, 0);
    msg << collection.c_str() << bson::X00;
    bson::Element::encode(msg, static_cast<int>(rm_one));
    bson::Element::encode(msg, selector);
    _encode_header(header, static_cast<int>(msg.tellp()), DELETE);
    _msg_send(header.str() + msg.str());
    return;
  }
  
  bson::Document MongoClient::runCommand(const std::string & dbname, const bson::Document & cmd)
  {
    return findOne(dbname + ".$cmd", cmd);
  }
}