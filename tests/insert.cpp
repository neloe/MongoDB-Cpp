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

TEST_F (MongoDriverTest, InsertOne)
{
    int thing = 0;
    while (c.findOne (INSCOLL, {{"a", thing}}).field_names().size() != 0)
    thing ++;
    c.insert (INSCOLL, {{"a", thing}});
    ASSERT_NE (0, c.findOne (INSCOLL, {{"a", thing}}).field_names().size());
}

TEST_F (MongoDriverTest, InsertOneNewConstructor)
{
    int thing = 0;
    mongo::MongoClient c2 (HOST, PORT, c.getContext().get());
    while (c2.findOne (INSCOLL, {{"a", thing}}).field_names().size() != 0)
    thing ++;
    c2.insert (INSCOLL, {{"a", thing}});
    ASSERT_NE (0, c.findOne (INSCOLL, {{"a", thing}}).field_names().size());
}

