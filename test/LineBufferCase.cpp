#include <boost/test/unit_test.hpp>

#include <io/Input.h>
#include <shell/LineBuffer.h>

using namespace std;

BOOST_AUTO_TEST_CASE( lineBuffer ) {
  LineBuffer lineBuffer{};

  BOOST_CHECK_EQUAL(lineBuffer.idx(), 0);
  BOOST_CHECK_EQUAL(lineBuffer.line(), "");
  BOOST_CHECK_EQUAL(lineBuffer.firstWord(), "");

  {
    string line{"testtest"};
    lineBuffer.line(line);

    BOOST_CHECK_EQUAL(lineBuffer.idx(), line.size());
    BOOST_CHECK_EQUAL(lineBuffer.line(), line);
    BOOST_CHECK_EQUAL(lineBuffer.firstWord(), line);
    lineBuffer.clear();
    BOOST_CHECK_EQUAL(lineBuffer.line(), "");
  }

  {
    string line{"test test"};
    lineBuffer.line(line);

    BOOST_CHECK_EQUAL(lineBuffer.idx(), line.size());
    BOOST_CHECK_EQUAL(lineBuffer.line(), line);
    BOOST_CHECK_EQUAL(lineBuffer.firstWord(), "test");
  }
}
