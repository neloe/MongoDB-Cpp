MongoDB-Cpp
===========

A replacement for the mongo-cxx-driver that focuses on memory safety, speed, and ease of use. It uses the FastBSON library (https://github.com/Lumate/FastBSON-Cpp) to handle decoding and encoding BSON.

## Prerequisites
This database driver requires the ZMQ libraries (and associated dependencies) and FastBSON. Build system requirements are CMake and a modern C++ compiler that is C++11 compliant.

## Build Instructions
When the test directories are submodules linking to other repositories.  In order to have all the required files needed to build the tests, make sure you initialize the submodules: `git submodule update --init --recursive` (or clone recursively).

This software uses CMake as it's build system; as such out of source builds are preferred.

You should create a build directory in the source directory and cd to it.

To prepare for release (from the build directory) :
`cmake .. && make mongocpp`

To run code coverage:
`cmake .. -DCMAKE_BUILD_TYPE=Debug && make mongo_coverage`

To generate Cobertura xml output for code coverage:
`cmake .. -DCMAKE_BUILD_TYPE=Debug && make mongo_cobertura`

## Progress
This is an evolving work in progress; it implements the MongoDB wire protocol, and over time will evolve in both ease of use and power.  The checklist at http://docs.mongodb.org/meta-driver/latest/legacy/mongodb-driver-requirements/ is used as a base.

### What works?

* CRUD Operations (Create, remove, update, destroy)
* Database commands (through the `runCommand` function)
* BSON (de/en)coding
* Cursor support
* closing exhausted cursors

### What's left?

* Authentication (not on any priority list)
* Simplified wrappers around common commands
* Graceful error handling (getting last error works, using it...)
* hint(), explain(), count(), $where (se second todo)
* profiling
* replica sets/slave OK, etc
* automatic reconnection (unsure what/how this means)

## Getting Started
All database functions take additional parameters.  See the function documentation for more information.  In all examples, initialization lists of intitialization lists are `bson::Document`s
### Creating a connection and connecting
```c++
mongo::MongoClient c;
c.connect("localhost"); //port is an optional string
// or
mongo::MongoClient c2("localhost", "27017"); //Again, port is optional
```
### Finding elements in the DB
#### Finding a single element
```c++
const std::string FINDCOLL = "database.collection"
bson::Document d = c.findOne(FINDCOLL); // Find any single
bson::Document d = c.findOne(FINDCOLL, {{"a", 5}}); // Find one element matches {"a": 5}, second param is a bson::Document
bson::Document d = c.findOne(FINDCOLL, {{"a", 5}}, {{"a", 1}}); // Same as above, but only include the "a" field in the doc
bson::Document d = c.findOne(FINDCOLL, {{"a", 5}}, {{"a", 0}}); // Same as above, omit the "a" field (projections)
```
#### Finding all elements that matches
```c++
mongo::Cursor curr = c.find(FINDCOLL);
while (curr.more())
{
  bson::Document d =  curr.next();
}
```
Finding all elements takes the same parameters as findOne.  See the function documentation for more details on the parameters
### Inserting elements
```c++
const std::string INSCOLL = "database.collection";
c.insert(INSCOLL, {{"a", thing}});
```
Batch inserting is currently not supported (future work)
### Removing elements
```c++
const std::string REMCOLL = "database.collection";
c.remove(REMCOLL, {{"a", thing}}); //remove a single element in the database that matches
c.remove(REMCOLL, {{"a", thing}}, true); //same as above
c.remove(REMCOLL, {{"a", thing}}, false); //Remove all elements that match
```
### Updating elements
```c++
const std::string UPDCOLL = "database.collection";
c.update(UPDCOLL, {{"a", oldthing}}, {{"a", thing}});
```
### Running database commands
```c++
const std::string DBNAME = "database";
bson::Document d = c.runCommand(DBNAME, {{"getLastError", 1}}); //Second parameter is formatted command
```
Commonly used commands will be wrapped in helper functions as progress continues
