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
  srand(std::time(NULL));
  int thing = rand() % 1000;
  while (c.findOne(COLL, {{"a", thing}}).field_names().size()!=0)
    thing = rand() % 1000;
  c.insert(COLL, {{"a",thing}});
  ASSERT_NE(0, c.findOne(COLL, {{"a", thing}}).field_names().size());
  c.remove(COLL, {{"a", thing}});
  ASSERT_EQ(0, c.findOne(COLL, {{"a", thing}}).field_names().size());
}