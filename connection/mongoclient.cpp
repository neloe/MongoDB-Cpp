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
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>

namespace mongo
{
    int MongoClient::m_req_id = 1;

    thread_local std::unordered_map<std::string, int> MongoClient::m_socks;

    MongoClient::MongoClient (): m_sock (-1) {}
    MongoClient::MongoClient (const std::string &host, const int port)
    {
        connect (host, port);
    }

    void MongoClient::connect (const std::string &host, const int port)
    {
        m_sock = socket(AF_INET, SOCK_STREAM, 0);
        struct hostent * server = gethostbyname(host.c_str());
        struct sockaddr_in serv_addr;
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
        serv_addr.sin_port = htons(port);
        ::connect(m_sock,(struct sockaddr *) &serv_addr,sizeof(serv_addr));
        /*
        std::string connstr = "tcp://" + host + ":" + port;
        if (m_socks.count (connstr) == 0)
        {
            m_socks[connstr] = std::make_shared<zmq::socket_t> (zmq::socket_t (*m_context, ZMQ_STREAM));
            m_socks[connstr]->connect (connstr.c_str());
        }
        m_sock = m_socks[connstr];
        m_id_size = _ID_MAX_SIZE;
        m_sock->getsockopt (ZMQ_IDENTITY, m_id, &m_id_size);*/
        return;
    }

    MongoClient::~MongoClient()
    {
    }

    void MongoClient::_msg_send (std::string message)
    {
        ::write(m_sock, message.c_str(), message.size());
        /*zmq::message_t send (message.size());
        std::memcpy ((void *)send.data(), message.c_str(), message.size());
        m_sock->send (m_id, m_id_size, ZMQ_SNDMORE);
        m_sock->send (send, ZMQ_SNDMORE);*/
    }
    void MongoClient::_msg_recv (reply_pre &intro, std::shared_ptr<unsigned char> &docs)
    {
        //zmq::message_t id, reply;
        int consumed, goal, size;
        //m_sock->recv (&id);
        //m_sock->recv (&reply);
        ::read(m_sock, (void*) &intro, REPLYPRE_SIZE);
        /*memcpy (& (intro.head), (char *)reply.data(), HEAD_SIZE);
        memcpy (& (intro.rFlags), (char *)reply.data() + HEAD_SIZE, sizeof (int));
        memcpy (& (intro.curID), (char *)reply.data() + HEAD_SIZE + sizeof (int), sizeof (long));
        memcpy (& (intro.start), (char *)reply.data() + HEAD_SIZE + sizeof (int) + sizeof (long), sizeof (int));
        memcpy (& (intro.numRet), (char *)reply.data() + HEAD_SIZE + sizeof (int) + sizeof (long)  + sizeof (int),
                sizeof (int));*/
        goal = intro.head.len - REPLYPRE_SIZE;
        
        docs = std::shared_ptr<unsigned char> (new unsigned char [intro.head.len - REPLYPRE_SIZE], [] (unsigned char *p)
        {
            delete[] p;
        });
        //read(m_sock, (void*) docs.get(), goal);
        //memcpy (docs.get(), (char *)reply.data() + REPLYPRE_SIZE, reply.size() - REPLYPRE_SIZE);
        consumed = REPLYPRE_SIZE;
        while (consumed < goal)
        {
            consumed += read(m_sock, (void*) docs.get(), goal - consumed);
            /*zmq::message_t intid, intreply;
            m_sock->recv (&intid);
            m_sock->recv (&intreply);
            memcpy (docs.get() + consumed, (char *)intreply.data(), std::min (static_cast<int> (intreply.size()), goal - consumed));
            consumed += std::min (static_cast<int> (intreply.size()), goal - consumed);*/
        }
        return;
    }
    void MongoClient::_kill_cursor (const long cursorID)
    {
        std::ostringstream kill;
        _encode_header (kill, HEAD_SIZE, KILL_CURSORS);
        bson::Element::encode (kill, 0);
        bson::Element::encode (kill, 1);
        bson::Element::encode (kill, cursorID);
        _msg_send (kill.str());
    }

    int MongoClient::_encode_header (std::ostringstream &ss, const int size, const int type)
    {
        int mid = m_req_id++;
        bson::Element::encode (ss, size + HEAD_SIZE);
        bson::Element::encode (ss, mid);
        bson::Element::encode (ss, 0);
        bson::Element::encode (ss, type);
        return mid;
    }

    void MongoClient::_more_cursor (Cursor &c)
    {
        reply_pre intro;
        std::ostringstream ss;
        _encode_header (ss, 17 + c.m_coll.size(), GET_MORE);
        bson::Element::encode (ss, 0);
        ss << c.m_coll.c_str() << bson::X00;
        bson::Element::encode (ss, 0);
        bson::Element::encode (ss, c.m_id);
        _msg_send (ss.str());
        _msg_recv (intro, c.m_docs);
        c.m_id = intro.curID;
        c.m_strsize = intro.head.len - REPLYPRE_SIZE;
        c.m_lastpos = 0;
        return;
    }

    void MongoClient::_msg_recv(MongoClient::reply_pre& intro, std::shared_ptr< unsigned char >& docs, const int id)
    {
        do
        {
            _msg_recv(intro, docs);
            if (intro.head.reTo != id)
                m_asyncs.push({intro, docs});
        } while (intro.head.reTo != id);
    }

    bson::Document MongoClient::findOne (const std::string &collection, const bson::Document &query,
                                         const bson::Document &projection, const int flags, const int skip)
    {
        std::ostringstream querystream, header;
        std::shared_ptr<unsigned char> data;
        reply_pre intro;
        bson::Document result;
        int mid;
        _format_find(collection, query, projection, flags, skip, 1, querystream);
        mid = _encode_header (header, static_cast<int> (querystream.tellp()), QUERY);
        _msg_send (header.str() + querystream.str());
        _msg_recv (intro, data, mid);
        
        if (intro.numRet == 1)
        {
            bson::Element e;
            e.decode (data.get(), bson::DOCUMENT);
            result = e.data<bson::Document>();
        }
        if (intro.curID != 0)
            _kill_cursor (intro.curID);
        return result;
    }

    Cursor MongoClient::find (const std::string &collection, const bson::Document &query,
                              const bson::Document &projection, const int flags, const int skip)
    {
        std::ostringstream querystream, header;
        std::shared_ptr<unsigned char> data;
        reply_pre intro;
        int mid;
        _format_find(collection, query, projection, flags, skip, 0, querystream);
        mid = _encode_header (header, static_cast<int> (querystream.tellp()), QUERY);
        _msg_send (header.str() + querystream.str());
        _msg_recv (intro, data);
        return Cursor (intro.curID, data, intro.head.len - REPLYPRE_SIZE, collection, *this);
    }

    int MongoClient::dispatch_find(const std::string& collection, const bson::Document& query, 
                                   const bson::Document& projection, const int flags, const int skip, const int limit)
    {
        std::ostringstream querystream, header;
        std::shared_ptr<unsigned char> data;
        reply_pre intro;
        int mid;
        _format_find(collection, query, projection, flags, skip, 0, querystream);
        mid = _encode_header (header, static_cast<int> (querystream.tellp()), QUERY);
        _msg_send (header.str() + querystream.str());
        return mid;
    }

    int MongoClient::dispatch_findOne(const std::string& collection, const bson::Document& query, 
                                      const bson::Document& projection, const int flags, const int skip)
    {
        return dispatch_find(collection, query, projection, flags, skip, 1);
    }

    int MongoClient::async_recv(bson::Document& result)
    {
        reply_pre intro;
        std::shared_ptr<unsigned char> data;
        if (m_asyncs.size())
        {
            intro = m_asyncs.front().intro;
            data = m_asyncs.front().data;
            m_asyncs.pop();
        }
        else 
            _msg_recv(intro, data);
        if (intro.numRet == 1)
        {
            bson::Element e;
            e.decode (data.get(), bson::DOCUMENT);
            e.data(result);
        }
        else
        {
            Cursor c (intro.curID, data, intro.head.len - REPLYPRE_SIZE, "", *this);
            if (c.more())
                result = c.next();
        }
        if (intro.curID != 0)
            _kill_cursor (intro.curID);
        return intro.head.reTo;
    }
    
    int MongoClient::async_recv(Cursor& result)
    {
        reply_pre intro;
        std::shared_ptr<unsigned char> data;
        _msg_recv(intro, data);
        result = Cursor (intro.curID, data, intro.head.len - REPLYPRE_SIZE, "", *this);
        return intro.head.reTo;
    }


    
    void MongoClient::_format_find(const std::string& collection, const bson::Document& query, 
                                   const bson::Document& projection, const int flags, const int skip, 
                                   const int limit, std::ostringstream& querystream)
    {
        bson::Document qd;
        //zmq::message_t reply;
        int num_returned;
        int docsize, headsize;
        qd.add ("$query", query);
        bson::Element::encode (querystream, flags);
        querystream << collection.c_str() << bson::X00;
        bson::Element::encode (querystream, skip);
        bson::Element::encode (querystream, limit);
        bson::Element::encode (querystream, qd);
        if (projection.field_names().size() > 0)
            bson::Element::encode (querystream, projection);
    }

    
    bool MongoClient::update (const std::string &collection, const bson::Document &selector, const bson::Document &update,
                              const bool upsert, const bool multi, const int timeout)
    {
        std::ostringstream msg, header;
        bson::Element::encode (msg, 0);
        msg << collection.c_str() << bson::X00;
        bson::Element::encode (msg, static_cast<int> (upsert) | (static_cast<int> (multi) << 1));
        bson::Element::encode (msg, selector);
        bson::Element::encode (msg, update);
        _encode_header (header, static_cast<int> (msg.tellp()), UPDATE);
        _msg_send (header.str() + msg.str());
        // try to get ack?
        bson::Document ack = _get_ack(collection, timeout);
        return ack.field_names().count("wtimeout") == 0 || ack["wtimeout"].data<bool>() == false;
    }

    bool MongoClient::insert (const std::string &collection, const bson::Document &toinsert, const int timeout)
    {
        std::ostringstream msg, header;
        bson::Element::encode (msg, 0);
        msg << collection.c_str() << bson::X00;
        bson::Element::encode (msg, toinsert);
        _encode_header (header, static_cast<int> (msg.tellp()), INSERT);
        _msg_send (header.str() + msg.str());
        bson::Document ack = _get_ack(collection, timeout);
        return ack.field_names().count("wtimeout") == 0 || ack["wtimeout"].data<bool>() == false;;
    }
    
    bson::Document MongoClient::_get_ack(const std::string & collection, const int timeout)
    {
        std::string db = collection.substr(0, collection.find("."));
        return runCommand(db, {{"getLastError", 1}, {"w", 1}, {"wtimeout", timeout}});
    }

    void MongoClient::remove (const std::string &collection, const bson::Document &selector, const bool rm_one)
    {
        std::ostringstream msg, header;
        bson::Element::encode (msg, 0);
        msg << collection.c_str() << bson::X00;
        bson::Element::encode (msg, static_cast<int> (rm_one));
        bson::Element::encode (msg, selector);
        _encode_header (header, static_cast<int> (msg.tellp()), DELETE);
        _msg_send (header.str() + msg.str());
        return;
    }

    bson::Document MongoClient::runCommand (const std::string &dbname, const bson::Document &cmd)
    {
        return findOne (dbname + ".$cmd", cmd);
    }
}