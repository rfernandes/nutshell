#include <boost/test/unit_test.hpp>

#include <shell/History.h>

using namespace std;

BOOST_AUTO_TEST_CASE( history ) {
  History history{};

  Line command{"test --command"};
  history.commandExecute(command);
  ParseResult result;
  history.commandExecuted(result, command);

  auto items = history.list();

  BOOST_CHECK_EQUAL(items.size(), 1);
  BOOST_CHECK_EQUAL(items[0].line, command);
}

