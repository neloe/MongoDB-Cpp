/*!
 * \file insert.cpp
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

TEST_F(MongoDriverTest, InsertOne)
{
  int thing = 0;
  while (c.findOne(INSCOLL, {{"a", thing}}).field_names().size() != 0)
    thing ++;
  c.insert(INSCOLL, {{"a", thing}});
  ASSERT_NE(0, c.findOne(INSCOLL, {{"a", thing}}).field_names().size());
}

TEST_F(MongoDriverTest, InsertOneNewConstructor)
{
  int thing = 0;
  mongo::MongoClient c2(HOST, PORT, c.getContext().get());
  while (c2.findOne(INSCOLL, {{"a", thing}}).field_names().size() != 0)
    thing ++;
  c2.insert(INSCOLL, {{"a", thing}});
  ASSERT_NE(0, c.findOne(INSCOLL, {{"a", thing}}).field_names().size());
}

