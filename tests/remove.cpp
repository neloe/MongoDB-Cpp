/*
  Copyright (c) Nathan Eloe, 2014
  This file is part of MongoDB-Cpp.

  MongoDB-Cpp is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  MongoDB-Cpp is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with MongoDB-Cpp.  If not, see <http://www.gnu.org/licenses/>.
*/
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

TEST_F (MongoDriverTest, RemoveOne)
{
    int thing = 0;
    while (c.findOne (REMCOLL, {{"a", thing}}).field_names().size() != 0)
    thing ++;
    c.insert (REMCOLL, {{"a", thing}});
    ASSERT_NE (0, c.findOne (REMCOLL, {{"a", thing}}).field_names().size());
    c.remove (REMCOLL, {{"a", thing}});
    ASSERT_EQ (0, c.findOne (REMCOLL, {{"a", thing}}).field_names().size());
}

TEST_F (MongoDriverTest, RemoveOneManyMatch)
{
    int thing = 0;
    while (c.findOne (REMCOLL, {{"a", thing}}).field_names().size() != 0)
    thing ++;
    c.insert (REMCOLL, {{"a", thing}, {"b", 5}});
    c.insert (REMCOLL, {{"a", thing}, {"b", 1}});
    ASSERT_NE (0, c.findOne (REMCOLL, {{"a", thing}}).field_names().size());
    c.remove (REMCOLL, {{"a", thing}});
    ASSERT_NE (0, c.findOne (REMCOLL, {{"a", thing}}).field_names().size());
}

TEST_F (MongoDriverTest, RemoveAllManyMatch)
{
    int thing = 0;
    while (c.findOne (REMCOLL, {{"a", thing}}).field_names().size() != 0)
    thing ++;
    c.insert (REMCOLL, {{"a", thing}, {"b", 5}});
    c.insert (REMCOLL, {{"a", thing}, {"b", 1}});
    ASSERT_NE (0, c.findOne (REMCOLL, {{"a", thing}}).field_names().size());
    c.remove (REMCOLL, {{"a", thing}}, false);
    ASSERT_EQ (0, c.findOne (REMCOLL, {{"a", thing}}).field_names().size());
}