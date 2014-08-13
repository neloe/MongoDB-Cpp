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