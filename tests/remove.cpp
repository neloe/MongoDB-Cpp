/*!
 * \file remove.cpp
 * \author Nathan Eloe
 * \brief tests find and findOne functionality of the mongo cxx driver
 */

#include "../connection/mongoclient.h"
#include "../connection/cursor.h"
#include "gtest/gtest.h"
#include "fixture.h"
#include <iostream>
#include <ctime>
#include <cstdlib>

TEST_F(MongoDriverTest, RemoveOne)
{
  int thing = 0;
  while (c.findOne(REMCOLL, {{"a", thing}}).field_names().size() != 0)
    thing ++;
  c.insert(REMCOLL, {{"a", thing}});
  ASSERT_NE(0, c.findOne(REMCOLL, {{"a", thing}}).field_names().size());
  c.remove(REMCOLL, {{"a", thing}});
  ASSERT_EQ(0, c.findOne(REMCOLL, {{"a", thing}}).field_names().size());
}

TEST_F(MongoDriverTest, RemoveOneManyMatch)
{
  int thing = 0;
  while (c.findOne(REMCOLL, {{"a", thing}}).field_names().size() != 0)
    thing ++;
  c.insert(REMCOLL, {{"a", thing}, {"b", 5}});
  c.insert(REMCOLL, {{"a", thing}, {"b", 1}});
  ASSERT_NE(0, c.findOne(REMCOLL, {{"a", thing}}).field_names().size());
  c.remove(REMCOLL, {{"a", thing}});
  ASSERT_NE(0, c.findOne(REMCOLL, {{"a", thing}}).field_names().size());
}

TEST_F(MongoDriverTest, RemoveAllManyMatch)
{
  int thing = 0;
  while (c.findOne(REMCOLL, {{"a", thing}}).field_names().size() != 0)
    thing ++;
  c.insert(REMCOLL, {{"a", thing}, {"b", 5}});
  c.insert(REMCOLL, {{"a", thing}, {"b", 1}});
  ASSERT_NE(0, c.findOne(REMCOLL, {{"a", thing}}).field_names().size());
  c.remove(REMCOLL, {{"a", thing}}, false);
  ASSERT_EQ(0, c.findOne(REMCOLL, {{"a", thing}}).field_names().size());
}