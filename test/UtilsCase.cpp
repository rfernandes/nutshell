#include <boost/test/unit_test.hpp>

#include <shell/Utils.h>

#include <sstream>

using namespace std;

BOOST_AUTO_TEST_CASE( utils_utf8 ) {
  using namespace utf8;

  //TODO: set locale
  BOOST_CHECK_EQUAL(bytes(0), 1);
  BOOST_CHECK_EQUAL(bytes('a'), 1);
  const char pound[] {"£"};
  BOOST_CHECK_EQUAL(bytes(pound[0]), 2);
}

enum class TestEnum{alpha, beta, gamma};

template<>
constexpr const char* enum_data<TestEnum>[] {"alpha", "beta", "gamma"};

ostream& operator << (ostream& out, TestEnum e)
{
  return out << enum_cast<const char *>(e);
}

BOOST_AUTO_TEST_CASE( utils_enum_cast ) {
  BOOST_CHECK_EQUAL("alpha", enum_cast<const char*>(TestEnum::alpha));
  BOOST_CHECK_EQUAL("beta", enum_cast<const char*>(TestEnum::beta));
  BOOST_CHECK_EQUAL("gamma", enum_cast<const char*>(TestEnum::gamma));

  BOOST_CHECK_EQUAL(TestEnum::alpha, enum_cast<TestEnum>("alpha"));
  BOOST_CHECK_EQUAL(TestEnum::beta, enum_cast<TestEnum>("beta"));
  BOOST_CHECK_EQUAL(TestEnum::gamma, enum_cast<TestEnum>("gamma"));
}
