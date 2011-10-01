#include "tokenizer.h"

#include <string>

#include <iostream>

#define BOOST_TEST_MODULE ColTest
#include <boost/test/unit_test.hpp>


/*Tests main class functionality*/

BOOST_AUTO_TEST_CASE(TestCaseOne) {
  // This test could probably be trimmed down without losing value
  int i;
  std::string test[] = {"a","a,b","a,b;c",",;,;,,,;;a",
    "a,,,,;;;;,;,;,;,", ";,a,,b,,c;;d,;"};
  std::string del[] = {",",";",",;"};

  std::vector<std::string> tokens;

  // Can we do basic tokenizing?
  tokenizeString(test[0], del[0], tokens);
  BOOST_REQUIRE(tokens.size() == 1);
  BOOST_REQUIRE(tokens[0] == "a");

  tokens.clear();
  tokenizeString(test[1], del[0], tokens);
  BOOST_REQUIRE(tokens.size() == 2);
  BOOST_REQUIRE(tokens.front() == "a");
  BOOST_REQUIRE(tokens.back() == "b");

  tokens.clear();
  tokenizeString(test[2], del[0], tokens);
  BOOST_REQUIRE(tokens.size() == 2);
  BOOST_REQUIRE(tokens.front() == "a");
  BOOST_REQUIRE(tokens.back() == "b;c");

  tokens.clear();
  tokenizeString(test[3], del[0], tokens);
  BOOST_REQUIRE(tokens.size() == 3);
  BOOST_REQUIRE(tokens[0] == ";" && tokens[1] == ";");
  BOOST_REQUIRE(tokens[2] == ";;a");
  
  tokens.clear();
  tokenizeString(test[4], del[0], tokens);
  BOOST_REQUIRE(tokens.size() == 5);
  BOOST_REQUIRE(tokens[0] == "a");
  BOOST_REQUIRE(tokens[1] == ";;;;");
  for( i = 2; i < 5; ++i )
    BOOST_REQUIRE( tokens[i] == ";" );

  tokens.clear();
  tokenizeString(test[5], del[0], tokens);
  BOOST_REQUIRE(tokens.size() == 5);
  BOOST_REQUIRE(tokens[0] == ";");
  BOOST_REQUIRE(tokens[1] == "a");
  BOOST_REQUIRE(tokens[2] == "b");
  BOOST_REQUIRE(tokens[3] == "c;;d");
  BOOST_REQUIRE(tokens[4] == ";");

  // And with a different delimeter?
  tokens.clear();
  tokenizeString(test[0], del[1], tokens);
  BOOST_REQUIRE(tokens.size() == 1);
  BOOST_REQUIRE(tokens[0] == "a");

  tokens.clear();
  tokenizeString(test[1], del[1], tokens);
  BOOST_REQUIRE(tokens.size() == 1);
  BOOST_REQUIRE(tokens.front() == "a,b");

  tokens.clear();
  tokenizeString(test[2], del[1], tokens);
  BOOST_REQUIRE(tokens.size() == 2);
  BOOST_REQUIRE(tokens.front() == "a,b");
  BOOST_REQUIRE(tokens.back() == "c");

  tokens.clear();
  tokenizeString(test[3], del[1], tokens);
  BOOST_REQUIRE(tokens.size() == 4);
  BOOST_REQUIRE(tokens[0] == "," && tokens[1] == ",");
  BOOST_REQUIRE(tokens[2] == ",,,");
  BOOST_REQUIRE(tokens[3] == "a");
  
  tokens.clear();
  tokenizeString(test[4], del[1], tokens);
  BOOST_REQUIRE(tokens.size() == 5);
  BOOST_REQUIRE(tokens[0] == "a,,,,");
  for( i = 1; i < 5; ++i )
    BOOST_REQUIRE( tokens[i] == "," );

  tokens.clear();
  tokenizeString(test[5], del[1], tokens);
  BOOST_REQUIRE(tokens.size() == 2);
  BOOST_REQUIRE(tokens[0] == ",a,,b,,c");
  BOOST_REQUIRE(tokens[1] == "d,");

  // With multiple delimeters?
  tokens.clear();
  tokenizeString(test[0], del[2], tokens);
  BOOST_REQUIRE(tokens.size() == 1);
  BOOST_REQUIRE(tokens[0] == "a");

  tokens.clear();
  tokenizeString(test[1], del[2], tokens);
  BOOST_REQUIRE(tokens.size() == 2);
  BOOST_REQUIRE(tokens.front() == "a");
  BOOST_REQUIRE(tokens.back() == "b");

  tokens.clear();
  tokenizeString(test[2], del[2], tokens);
  BOOST_REQUIRE(tokens.size() == 3);
  BOOST_REQUIRE(tokens[0] == "a");
  BOOST_REQUIRE(tokens[1] == "b");
  BOOST_REQUIRE(tokens[2] == "c");

  tokens.clear();
  tokenizeString(test[3], del[2], tokens);
  BOOST_REQUIRE(tokens.size() == 1);
  BOOST_REQUIRE(tokens.front() == "a");
  
  tokens.clear();
  tokenizeString(test[4], del[2], tokens);
  BOOST_REQUIRE(tokens.size() == 1);
  BOOST_REQUIRE(tokens.front() == "a");

  tokens.clear();
  tokenizeString(test[5], del[2], tokens);
  BOOST_REQUIRE(tokens.size() == 4);
  BOOST_REQUIRE(tokens[0] == "a");
  BOOST_REQUIRE(tokens[1] == "b");
  BOOST_REQUIRE(tokens[2] == "c");
  BOOST_REQUIRE(tokens[3] == "d");

  // Test skipping ahead some number of characters
  tokens.clear();
  tokenizeString(test[3],del[0],tokens,4);
  BOOST_REQUIRE(tokens.size() == 1);
  BOOST_REQUIRE(tokens[0] == ";;a");

  // Test stopping on a specific character
  tokens.clear();
  tokenizeString(test[2],del[0],tokens,0,";");
  BOOST_REQUIRE(tokens.size() == 2); 

  // Test stopping after some number of tokens found
  tokens.clear();
  tokenizeString(test[5], del[2], tokens, 0, "", 2);
  BOOST_REQUIRE(tokens.size() == 2);
  BOOST_REQUIRE(tokens[0] == "a");
  BOOST_REQUIRE(tokens[1] == "b");

  // Test saving last token when requested
  tokens.clear();
  tokenizeString(test[4],del[2],tokens,0,";",0,true);
  BOOST_REQUIRE(tokens.size() == 2);
  BOOST_REQUIRE(tokens.front() == "a");
  BOOST_REQUIRE(tokens.back() == ";;;;,;,;,;,");

  // Test empty strings
  tokens.clear();
  tokenizeString("",del[0], tokens);
  BOOST_REQUIRE(tokens.size() == 0);

  tokens.clear();
  tokenizeString(test[5], "", tokens);
  BOOST_REQUIRE(tokens.size() == 1 );

  // We don't need to test any arguments with defaults, as we know those work,
  // by tests that don't specify values above.
}


