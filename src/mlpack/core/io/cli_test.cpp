/***
 * @file io_test.cc
 * @author Matthew Amidon, Ryan Curtin
 *
 * Test for the CLI input parameter system.
 */
#include "optionshierarchy.h"
#include "cli.hpp"
#include "../io/log.h"

#include <mlpack/core.h>
#include <iostream>
#include <sstream>
#include <sys/time.h>

#define DEFAULT_INT 42

#define BOOST_TEST_MODULE CLI_Test
#include <boost/test/unit_test.hpp>

#define BASH_RED "\033[0;31m"
#define BASH_GREEN "\033[0;32m"
#define BASH_YELLOW "\033[0;33m"
#define BASH_CYAN "\033[0;36m"
#define BASH_CLEAR "\033[0m"

using namespace mlpack;
using namespace mlpack::io;

/***
 * @brief Tests that inserting elements into an OptionsHierarchy
 *   properly updates the tree.
 *
 * @return True indicating all is well with OptionsHierarchy
 */
BOOST_AUTO_TEST_CASE(TestHierarchy) {
  OptionsHierarchy tmp = OptionsHierarchy("UTest");
  std::string testName = std::string("UTest/test");
  std::string testDesc = std::string("Test description.");
  std::string testTID = TYPENAME(int);

  // Check that the hierarchy is properly named.
  std::string str = std::string("UTest");
  OptionsData node = tmp.GetNodeData();

  BOOST_REQUIRE_EQUAL(str.compare(node.node), 0);
  // Check that inserting a node actually inserts the node.
  // Note, that since all versions of append simply call the most qualified
  //    overload, we will only test that one.
  tmp.AppendNode(testName, testTID, testDesc);
  BOOST_REQUIRE(tmp.FindNode(testName) != NULL);

  // Now check that the inserted node has the correct data.
  OptionsHierarchy* testHierarchy = tmp.FindNode(testName);
  OptionsData testData;
  if (testHierarchy != NULL) {
    node = testHierarchy->GetNodeData();

    BOOST_REQUIRE(testName.compare(node.node) == 0);
    BOOST_REQUIRE(testDesc.compare(node.desc) == 0);
    BOOST_REQUIRE(testTID.compare(node.tname) == 0);
  }
}

/***
 * @brief Tests that CLI works as intended, namely that CLI::Add
 *   propagates successfully.
 *
 * @return True indicating all is well with CLI::Add, false otherwise.
 */
BOOST_AUTO_TEST_CASE(TestCLIAdd) {
  // Check that the CLI::HasParam returns false if no value has been specified
  // on the commandline and ignores any programmatical assignments.
  CLI::Add<bool>("bool", "True or False", "global");
  BOOST_REQUIRE_EQUAL(CLI::HasParam("global/bool"), false);
  CLI::GetParam<bool>("global/bool") = true;
  // CLI::HasParam should return true.
  BOOST_REQUIRE_EQUAL(CLI::HasParam("global/bool"), true);

  //Check the description of our variable.
  BOOST_REQUIRE_EQUAL(CLI::GetDescription("global/bool").compare(
      std::string("True or False")) , 0);
  // Check that SanitizeString is sanitary.
  std::string tmp = CLI::SanitizeString("/foo/bar/fizz");
  BOOST_REQUIRE_EQUAL(tmp.compare(std::string("foo/bar/fizz/")),0);
}

/***
 * Test the output of CLI.  We will pass bogus input to a stringstream so that
 * none of it gets to the screen.
 */
BOOST_AUTO_TEST_CASE(TestPrefixedOutStreamBasic) {
  std::stringstream ss;
  PrefixedOutStream pss(ss, BASH_GREEN "[INFO ] " BASH_CLEAR);

  pss << "This shouldn't break anything" << std::endl;
  BOOST_REQUIRE_EQUAL(ss.str(),
      BASH_GREEN "[INFO ] " BASH_CLEAR "This shouldn't break anything\n");

  ss.str("");
  pss << "Test the new lines...";
  pss << "shouldn't get 'Info' here." << std::endl;
  BOOST_REQUIRE_EQUAL(ss.str(),
      BASH_GREEN "[INFO ] " BASH_CLEAR
      "Test the new lines...shouldn't get 'Info' here.\n");

  pss << "But now I should." << std::endl << std::endl;
  pss << "";
  BOOST_REQUIRE_EQUAL(ss.str(),
      BASH_GREEN "[INFO ] " BASH_CLEAR
      "Test the new lines...shouldn't get 'Info' here.\n"
      BASH_GREEN "[INFO ] " BASH_CLEAR "But now I should.\n"
      BASH_GREEN "[INFO ] " BASH_CLEAR "\n"
      BASH_GREEN "[INFO ] " BASH_CLEAR "");
}

/**
 * @brief Tests that the various PARAM_* macros work properly
 * @return True indicating that all is well with CLI & Options.
 */
BOOST_AUTO_TEST_CASE(TestOption) {
  // This test will involve creating an option, and making sure CLI reflects
  // this.
  PARAM(int, "test", "test desc", "test_parent", DEFAULT_INT, false);

  // Does CLI reflect this?
  BOOST_REQUIRE_EQUAL(CLI::HasParam("test_parent/test"), true);

  std::string desc = std::string("test desc");

  BOOST_REQUIRE_EQUAL(CLI::GetDescription("test_parent/test"), "test desc");
  BOOST_REQUIRE_EQUAL(CLI::GetParam<int>("test_parent/test"), DEFAULT_INT);
}

/***
 * Ensure that a Boolean option which we define is set correctly.
 */
BOOST_AUTO_TEST_CASE(TestBooleanOption) {
  PARAM_FLAG("flag_test", "flag test description", "test_parent");

  BOOST_REQUIRE_EQUAL(CLI::HasParam("test_parent/flag_test"), false);

  BOOST_REQUIRE_EQUAL(CLI::GetDescription("test_parent/flag_test"),
      "flag test description");

  // Now check that CLI reflects that it is false by default.
  BOOST_REQUIRE_EQUAL(CLI::GetParam<bool>("test_parent/flag_test"), false);
}


/***
 * Test that we can correctly output Armadillo objects to PrefixedOutStream
 * objects.
 */
BOOST_AUTO_TEST_CASE(TestArmadilloPrefixedOutStream) {
  // We will test this with both a vector and a matrix.
  arma::vec test("1.0 1.5 2.0 2.5 3.0 3.5 4.0");

  std::stringstream ss;
  PrefixedOutStream pss(ss, BASH_GREEN "[INFO ] " BASH_CLEAR);

  pss << test;
  // This should result in nothing being on the current line (since it clears
  // it).
  BOOST_REQUIRE_EQUAL(ss.str(), BASH_GREEN "[INFO ] " BASH_CLEAR "   1.0000\n"
                                BASH_GREEN "[INFO ] " BASH_CLEAR "   1.5000\n"
                                BASH_GREEN "[INFO ] " BASH_CLEAR "   2.0000\n"
                                BASH_GREEN "[INFO ] " BASH_CLEAR "   2.5000\n"
                                BASH_GREEN "[INFO ] " BASH_CLEAR "   3.0000\n"
                                BASH_GREEN "[INFO ] " BASH_CLEAR "   3.5000\n"
                                BASH_GREEN "[INFO ] " BASH_CLEAR "   4.0000\n");

  ss.str("");
  pss << trans(test);
  // This should result in there being stuff on the line.
  BOOST_REQUIRE_EQUAL(ss.str(), BASH_GREEN "[INFO ] " BASH_CLEAR
      "   1.0000   1.5000   2.0000   2.5000   3.0000   3.5000   4.0000\n");

  arma::mat test2("1.0 1.5 2.0; 2.5 3.0 3.5; 4.0 4.5 4.99999");
  ss.str("");
  pss << test2;
  BOOST_REQUIRE_EQUAL(ss.str(),
      BASH_GREEN "[INFO ] " BASH_CLEAR "   1.0000   1.5000   2.0000\n"
      BASH_GREEN "[INFO ] " BASH_CLEAR "   2.5000   3.0000   3.5000\n"
      BASH_GREEN "[INFO ] " BASH_CLEAR "   4.0000   4.5000   5.0000\n");

  // Try and throw a curveball by not clearing the line before outputting
  // something else.  The PrefixedOutStream should not force Armadillo objects
  // onto their own lines.
  ss.str("");
  pss << "hello" << test2;
  BOOST_REQUIRE_EQUAL(ss.str(),
      BASH_GREEN "[INFO ] " BASH_CLEAR "hello   1.0000   1.5000   2.0000\n"
      BASH_GREEN "[INFO ] " BASH_CLEAR "   2.5000   3.0000   3.5000\n"
      BASH_GREEN "[INFO ] " BASH_CLEAR "   4.0000   4.5000   5.0000\n");
}

/***
 * Test that we can correctly output things in general.
 */
BOOST_AUTO_TEST_CASE(TestPrefixedOutStream) {
  std::stringstream ss;
  PrefixedOutStream pss(ss, BASH_GREEN "[INFO ] " BASH_CLEAR);

  pss << "hello world I am ";
  pss << 7;

  BOOST_REQUIRE_EQUAL(ss.str(),
      BASH_GREEN "[INFO ] " BASH_CLEAR "hello world I am 7");

  pss << std::endl;
  BOOST_REQUIRE_EQUAL(ss.str(),
      BASH_GREEN "[INFO ] " BASH_CLEAR "hello world I am 7\n");

  ss.str("");
  pss << std::endl;
  BOOST_REQUIRE_EQUAL(ss.str(),
      BASH_GREEN "[INFO ] " BASH_CLEAR "\n");
}

/***
 * Test format modifiers.
 */
BOOST_AUTO_TEST_CASE(TestPrefixedOutStreamModifiers) {
  std::stringstream ss;
  PrefixedOutStream pss(ss, BASH_GREEN "[INFO ] " BASH_CLEAR);

  pss << "I have a precise number which is ";
  pss << std::setw(6) << std::setfill('0') << 156;

  BOOST_REQUIRE_EQUAL(ss.str(),
      BASH_GREEN "[INFO ] " BASH_CLEAR
      "I have a precise number which is 000156");
}
