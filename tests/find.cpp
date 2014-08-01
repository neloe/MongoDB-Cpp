/*!
 * \file find.cpp
 * \author Nathan Eloe
 * \brief tests find and findOne functionality of the mongo cxx driver
 */

#include "../connection/mongoclient.h"
#include "../connection/cursor.h"
#include "gtest/gtest.h"
#include <iostream>

TEST(Find, FindOneAny)
{
  mongo::MongoClient c("localhost");
  bson::Document d = c.findOne("test.testdata");
  ASSERT_NE(0, d.field_names().size());
}

TEST(Find, FindOneNoExist)
{
  mongo::MongoClient c("localhost");
  bson::Document d = c.findOne("test.testdata", {{"A", 1}});
  ASSERT_EQ(0, d.field_names().size());
}

TEST(Find, FindOneFilter)
{
  mongo::MongoClient c("localhost");
  bson::Document d = c.findOne("test.testdata", {{"a", 1}});
  ASSERT_EQ(1.0, d["a"].data<int>());
}

TEST(Find, FindOneProject)
{
  mongo::MongoClient c("localhost");
  bson::Document d = c.findOne("test.testdata", {{"a", 1}}, {{"a", 1}});
  ASSERT_GE(2, d.field_names().size()); //can still have the _id apparently... ugh
  ASSERT_EQ(1.0, d["a"].data<int>());
  ASSERT_EQ(1, d.field_names().count("a"));
  ASSERT_EQ(1, d.field_names().count("_id"));
  ASSERT_EQ(0, d.field_names().count("b"));
}

TEST(Find, FineOneProjectFilter)
{
  mongo::MongoClient c("localhost");
  bson::Document d = c.findOne("test.testdata", {{"a", 1}}, {{"a", 0}});
  ASSERT_NE(0, d.field_names().size());
  ASSERT_EQ(0, d.field_names().count("a"));
}

TEST(Find, FindAll)
{
  int count = 0;
  mongo::MongoClient c("localhost");
  mongo::Cursor curr = c.find("test.testdata");
  while (curr.more())
  {
    count++;
    curr.next();
  }
  ASSERT_LT(0, count);
}