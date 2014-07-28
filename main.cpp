#include "connection/basic_connection.h"
#include <iostream>
#include <string>
using namespace std;
int main()
{
  mongo::BasicConnection conn("localhost");
  cout << conn.findOne("a.b") << endl;
  return 0;
}