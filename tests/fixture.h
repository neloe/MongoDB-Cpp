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
#pragma once
#include "gtest/gtest.h"
#include <string>
#include "../connection/mongoclient.h"

const std::string HOST = "localhost";
const std::string PORT = "27017";

const std::string FINDCOLL = "mongo-driver-test.find";
const std::string INSCOLL  = "mongo-driver-test.insert";
const std::string REMCOLL  = "mongo-driver-test.remove";
const std::string UPDCOLL  = "mongo-driver-test.update";

class MongoDriverTest: public ::testing::Test
{
  protected:
    mongo::MongoClient c;
    virtual void SetUp()
    {
      c.connect(HOST, PORT);
    }    
};