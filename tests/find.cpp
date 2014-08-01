/*!
 * \file find.cpp
 * \author Nathan Eloe
 * \brief tests find and findOne functionality of the mongo cxx driver
 */

#include "../connection/mongoclient.h"
#include "../connection/cursor.h"
#include "gtest/gtest.h"
#include "fixture.h"
#include <iostream>

TEST_F(MongoDriverTest, FindOneAny)
{
  bson::Document d = c.findOne(COLL);
  ASSERT_NE(0, d.field_names().size());
}

TEST_F(MongoDriverTest, FindOneNoExist)
{
  bson::Document d = c.findOne(COLL, {{"A", 1}});
  ASSERT_EQ(0, d.field_names().size());
}

TEST_F(MongoDriverTest, FindOneFilter)
{
  bson::Document d = c.findOne(COLL, {{"a", 1}});
  ASSERT_EQ(1, d["a"].data<int>());
}

TEST_F(MongoDriverTest, FindOneProject)
{
  bson::Document d = c.findOne(COLL, {{"a", 1}}, {{"a", 1}});
  ASSERT_GE(2, d.field_names().size()); //can still have the _id apparently... ugh
  ASSERT_EQ(1, d["a"].data<int>());
  ASSERT_EQ(1, d.field_names().count("a"));
  ASSERT_EQ(1, d.field_names().count("_id"));
  ASSERT_EQ(0, d.field_names().count("b"));
}

TEST_F(MongoDriverTest, FineOneProjectFilter)
{
  bson::Document d = c.findOne(COLL, {{"a", 1}}, {{"a", 0}});
  ASSERT_NE(0, d.field_names().size());
  ASSERT_EQ(0, d.field_names().count("a"));
}

TEST_F(MongoDriverTest, FindAll)
{
  int count = 0;
  mongo::Cursor curr = c.find(COLL);
  while (curr.more())
  {
    count++;
    curr.next();
  }
  ASSERT_LT(0, count);
}