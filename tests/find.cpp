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
  bson::Document d = c.findOne(FINDCOLL);
  ASSERT_NE(0, d.field_names().size());
}

TEST_F(MongoDriverTest, FindOneNoExist)
{
  bson::Document d = c.findOne(FINDCOLL, {{"A", 1}});
  ASSERT_EQ(0, d.field_names().size());
}

TEST_F(MongoDriverTest, FindOneFilter)
{
  bson::Document d = c.findOne(FINDCOLL, {{"a", 5}});
  ASSERT_EQ(5, d["a"].data<int>());
}

TEST_F(MongoDriverTest, FindOneOnly)
{
  bson::Document d = c.findOne(FINDCOLL, {{"b", 8}});
  ASSERT_EQ(8, d["b"].data<int>());
}

TEST_F(MongoDriverTest, FindOneProject)
{
  bson::Document d = c.findOne(FINDCOLL, {{"a", 5}}, {{"a", 1}});
  ASSERT_GE(2, d.field_names().size()); //can still have the _id apparently... ugh
  ASSERT_EQ(5, d["a"].data<int>());
  ASSERT_EQ(1, d.field_names().count("a"));
  ASSERT_EQ(1, d.field_names().count("_id"));
  ASSERT_EQ(0, d.field_names().count("b"));
}

TEST_F(MongoDriverTest, FindOneProjectFilter)
{
  bson::Document d = c.findOne(FINDCOLL, {{"a", 5}}, {{"a", 0}});
  ASSERT_NE(0, d.field_names().size());
  ASSERT_EQ(0, d.field_names().count("a"));
}

TEST_F(MongoDriverTest, FindAll)
{
  int count = 0;
  mongo::Cursor curr = c.find(FINDCOLL);
  while (curr.more())
  {
    count++;
    curr.next();
  }
  ASSERT_EQ(10004, count);
}

TEST_F(MongoDriverTest, FindProjectFilter)
{
  int count = 0;
  mongo::Cursor curr = c.find(FINDCOLL, {{"a", 5}}, {{"a", 0}});
  while (curr.more())
  {
    count++;
    curr.next();
  }
  ASSERT_EQ(2, count);
}