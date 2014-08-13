/*!
 * \file cmd.cpp
 * \author Nathan Eloe
 * \brief tests runcommand functionality of the mongo cxx driver
 */

#include "../connection/mongoclient.h"
#include "../connection/cursor.h"
#include "gtest/gtest.h"
#include "fixture.h"
#include <iostream>

TEST_F(MongoDriverTest, GetLastError)
{
  bson::Document d = c.runCommand("mongo-driver-test", {{"getLastError", 1}});
  ASSERT_EQ(6, d.field_names().size());
}