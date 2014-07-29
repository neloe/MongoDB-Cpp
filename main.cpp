#include "connection/mongoclient.h"
#include "bson/document.h"
#include "bson/element.h"
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <set>
using namespace std;
int main()
{
  //mongo::BasicConnection conn("localhost");
  mongo::MongoClient conn;
  bson::array pt = {-91.8, 37.5};
  //cout << conn.findOne("a.b") << endl;
  bson::Document squery, geo, geom, geomint, result, projection, tap, demo, query2;
  std::string id;
  geo.add("type", std::string("Point"));
  geo.add("coordinates",pt);
  geom.add("$geometry", geo);
  geomint.add("$geoIntersects", geom);
  squery.add("geometry", geomint);
  cout << squery << endl;
  cout << geo << endl;
  projection.add("geometry",0);
  projection.add("NAME", 0);
  projection.add("ST_ABBREV", 0);
  //projection.add("ID", 0);
  
  result = conn.findOne("esri.blockgroups", squery, projection);
  result["ID"].data(id);
  query2.add("ID",id);
  int count = 0;
  projection.add("ID", 0);
  
  using namespace std::chrono;

  high_resolution_clock::time_point t1 = high_resolution_clock::now();
  result = conn.findOne("esri.blockgroups", query2, projection);
  tap = result["tapestry"].data<bson::Document>();
  demo = result["demographics"].data<bson::Document>();
  std::pair<std::string, std::string> pair;
  std::set<std::string> fields = tap.field_names();
  for (std::string f: fields)
  {
    pair.first = f;
    pair.second = tap[f].data<std::string>();
    count ++;
  }
  cout << "Tapestry: " << count << endl;
  count = 0;
  fields = demo.field_names();
  for (std::string f: fields)
  {
    pair.first = f;
    pair.second = demo[f].data<std::string>();
    count ++;
  }
  cout << "Demo: " << count << endl;
  high_resolution_clock::time_point t2 = high_resolution_clock::now();

  duration<double> time_span = duration_cast<duration<double>>(t2 - t1);

  std::cout << "It took me " << time_span.count() << " seconds.";
  std::cout << std::endl;
  return 0;
}