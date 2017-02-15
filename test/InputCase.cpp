#include <boost/test/unit_test.hpp>

#include <io/Cursor.h>

#include <sstream>

using namespace std;

BOOST_AUTO_TEST_CASE( input ) {
  stringstream in{""};
  stringstream out{""};

  Cursor cursor(in, out);
  cursor.column(1);
  BOOST_CHECK_EQUAL(out.str(), "\x1b[1G");
  out.str("");
  cursor.column(2);
  BOOST_CHECK_EQUAL(out.str(), "\x1b[2G");

  in.str("^[5;6R");
  out.str("");
  const auto& pos = cursor.position();
  BOOST_CHECK_EQUAL(out.str(), "\x1b[6n");
  BOOST_CHECK_EQUAL(pos.x, 6);
  BOOST_CHECK_EQUAL(pos.y, 5);
}
