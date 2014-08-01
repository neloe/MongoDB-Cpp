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

TEST_F(MongoDriverTest, Update)
{
  srand(std::time(NULL));
  int thing = rand() % 1000;
  int oldthing;
  while (c.findOne(COLL, {{"a", thing}}).field_names().size()!=0)
    thing = rand() % 1000;
  c.insert(COLL, {{"a",thing}});
  oldthing=thing;
  while (c.findOne(COLL, {{"a", thing}}).field_names().size()!=0)
    thing++;
  
  c.update(COLL, {{"a", oldthing}}, {{"a", thing}});
  ASSERT_EQ(0, c.findOne(COLL, {{"a", oldthing}}).field_names().size());
  ASSERT_NE(0, c.findOne(COLL, {{"a", thing}}).field_names().size());
}