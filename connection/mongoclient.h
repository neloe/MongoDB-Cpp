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
 * \file basic_connection.h
 * \author Nathan Eloe
 * \brief Declaration of a basic, naive connection to the database
 */

#pragma once
#include <memory>
#include <string>
#include <zmq.hpp>
#include <queue>
#include <utility>
#include <unordered_map>

#include "../bson/document.h"
#include "../bson/element.h"



namespace mongo
{
    class Cursor;

    const int HEAD_SIZE = 16;
    const int REPLYPRE_SIZE = 36;

    class MongoClient
    {
      private:
        //Internal types and codes
        friend class Cursor;
        //MongoDB codes
        enum opcodes {REPLY = 1, MSG = 1000, UPDATE = 2001, INSERT, RESERVED, QUERY, GET_MORE, DELETE, KILL_CURSORS};
        struct msg_header
        {
            int len, reqID, reTo, opCode;
        };
        struct reply_pre
        {
            msg_header head;
            int rFlags;
            long curID;
            int start, numRet;
        };
        struct async_reply
        {
            reply_pre intro;
            std::shared_ptr<unsigned char> data;
        };

        //The maximum size a TCP id can have
        const static size_t _ID_MAX_SIZE = 256;
        //The current request ID
        static int m_req_id;
        //The ZMQ context used for communication
        static std::shared_ptr<zmq::context_t> m_context;
        //The connection pool (ZMQ sockets)
        static thread_local std::unordered_map<std::string, std::shared_ptr<zmq::socket_t>> m_socks;
        //This client's socket
        std::shared_ptr<zmq::socket_t>m_sock;
        //The id of the server to send to
        char m_id[_ID_MAX_SIZE];
        //The size of the server's ID
        size_t m_id_size;
        //backed up asynchronous requests
        std::queue <async_reply> m_asyncs;

        /*!
         * \brief sends a message over the zmq socket (abstracts the messiness caused by using ZMQ_STREAM
         * \pre The internal zmq socket should be connected to a database
         * \post The message is sent over the internal zmq socket
         */
        void _msg_send (std::string message);
        /*!
         * \brief receives a message from the zmq socket (handles multiple frames)
         * \pre The internal zmq socket should be connected to a database and be expecting a response
         * \post A (possibly multi-framed) message is received and stored in the array of unsigned characters
         */
        void _msg_recv (reply_pre &intro, std::shared_ptr<unsigned char> &docs);
        /*!
         * \brief receives messages from the zmq socket (handles multiple frames) until we get the response to
         *        the specified message id
         * \pre The internal zmq socket should be connected to a database and be expecting a response
         * \post A (possibly multi-framed) message is received and stored in the array of unsigned characters
         */
        void _msg_recv (reply_pre &intro, std::shared_ptr<unsigned char> &docs, const int id);
        /*!
         * \brief Kills a database cursor
         * \pre The internal zmq socket should be connected to a database
         * \post sends the kill cursor message to the connected database
         */
        void _kill_cursor (const long cursorID);
        /*!
         * \brief encodes the common message header
         * \pre None
         * \post the message header is encoded in the output string stream
         * \return the message id
         */
        int _encode_header (std::ostringstream &ss, const int size, const int type);
        /*!
         * \brief gets more information for the cursor to load
         * \pre this connection should be the same one that created the cursor (same host/port pair)
         * \post iterates the database cursor
         */
        void _more_cursor (Cursor &c);
        /*!
         * \brief the common functionality behind encoding and sending requests
         * \pre None
         * \post populates the querystream and header ostringstreams with the header and request
         */
        void _format_find(const std::string &collection, const bson::Document &query,
                          const bson::Document &projection, const int flags,
                          const int skip, const int limit, std::ostringstream & querystream);
      public:
        /*!
         * \brief Constructors
         * \pre The context should be passed if it has been created for other ZMQ sockets
         * \post The client is constructed using the context specified.  If *ctx == nullptr, creates a new zmq context
         */
        MongoClient (zmq::context_t *ctx = nullptr);
        MongoClient (zmq::context_t &ctx): MongoClient (&ctx) {}
        /*!
         * \brief Connection Constructors
         * \pre None
         * \post constructs the client object and connects to the database
         */
        MongoClient (const std::string &host, const std::string &port = "27017", zmq::context_t *ctx = nullptr);
        MongoClient (zmq::context_t &ctx, const std::string &host, const std::string &port = "27017"): MongoClient (host, port,
                    &ctx) {}

        /*!
         * \brief Destructor
         * \pre None
         * \post Destructs and cleans up the object
         */
        ~MongoClient();

        /*!
         * \brief connects to the database
         * \pre None
         * \post creates a connection to the database at the specified host and port
         */

        void connect (const std::string &host, const std::string &port = "27017");

        // Database Operations (CRUD)

        /*!
         * \brief finds and returns a single document
         * \pre None
         * \post Performs a query on the database
         * \return a single matching Document; if no match, returns an empty Document
         */
        bson::Document findOne (const std::string &collection, const bson::Document &query = bson::Document(),
                                const bson::Document &projection = bson::Document(), const int flags = 0,
                                const int skip = 0);

        /*!
         * \brief Finds and returns a cursor to multiple documents
         * \pre None
         * \post Performs a query on the database
         * \return A Cursor object containing the database cursor to get data out of
         */
        Cursor find (const std::string &collection, const bson::Document &query = bson::Document(),
                     const bson::Document &projection = bson::Document(), const int flags = 0,
                     const int skip = 0);

        /*!
         * \brief dispatches a find operation (for asynchronous use) and returns a request ID
         * \pre None
         * \post Sends a find operation mesage to the database
         * \return the request ID
         */
        int dispatch_find(const std::string &collection, const bson::Document &query = bson::Document(),
                       const bson::Document &projection = bson::Document(), const int flags = 0,
                       const int skip = 0, const int limit = 0);
        
        int dispatch_findOne(const std::string &collection, const bson::Document &query = bson::Document(),
                          const bson::Document &projection = bson::Document(), const int flags = 0,
                          const int skip = 0);
        
        /*!
         * \brief recieves a result from a dispatched find or findOne
         * \pre at least one of the dispatch_* functions has been called
         * \post recieves a message from the database, and populates the reference parameter.
         * \return the request ID
         */
        int async_recv(bson::Document & result);
        int async_recv(Cursor & result);
        /*!
         * \brief Runs an update operation on the database
         * \pre None
         * \post Runs the update operation on the database
         */
        void update (const std::string &collection, const bson::Document &selector, const bson::Document &update,
                     const bool upsert = false, const bool multi = false);

        /*!
         * \brief Runs an insertion operation on the database
         * \pre None
         * \post Inserts the document into the database
         */
        void insert (const std::string &collection, const bson::Document &toinsert);

        /*!
         * \brief Runs a removal operation on the database
         * \pre None
         * \post Removes the specified selector from the database (defaults to removing a single element)
         */
        void remove (const std::string &collection, const bson::Document &selector, const bool rm_one = true);

        // Database Command functions

        /*!
         * \brief runs the specified database command
         * \pre None
         * \post Runs the database command
         * \return the resulting bson document from running the command
         */
        bson::Document runCommand (const std::string &dbname, const bson::Document &cmd);
        /*!
         * \brief runs the specified database command
         * \pre None
         * \post Runs the database command {cmd: args}
         * \return the resulting bson document from running the command
         */
        bson::Document runCommand (const std::string &dbname, const std::string &cmd, const bson::Element args = 1)
        {
            return runCommand (dbname, {{cmd, args}});
        }
        
        /*!
         * \brief gets the socket used by this mongo client
         * \pre connect has been called
         * \post None
         * \return a reference to the zmq socket
         */
        zmq::socket_t& getSocket()
        {
            return *(m_sock.get());
        }
        
        /*!
         * \brief gets the created context
         * \pre None
         * \post None
         * \return The context used to manage the ZMQ connections
         */
        static std::shared_ptr<zmq::context_t> getContext()
        {
            return m_context;
        }

        /*!
         * \brief sets the shared context for the mongo clients
         * \pre None
         * \post Sets the shared static context for the mongo client instances to the supplied context
         */
        static void setContext (zmq::context_t *ctx)
        {
            m_context = std::shared_ptr<zmq::context_t> (ctx);
        }
        static void setContext (zmq::context_t &ctx)
        {
            m_context = std::shared_ptr<zmq::context_t> (&ctx);
        }
    };
}