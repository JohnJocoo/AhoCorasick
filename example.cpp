#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

#include "aho_corasick.hpp"

int main(int argc, const char * argv[])
{
  std::string string{"quick red fox jumps over lazy brown dog that owns brown house"};
  // create instance
  AhoCorasick<std::string> automation;
  // add patterns to search for
  automation.addPattern("quick");
  automation.addPattern("ck");
  automation.addPattern("azy");
  automation.addPattern("own");
  automation.addPattern("brown");
  
  // get matcher object
  auto matcher = automation.matcher();
  
  // run search
  auto begin = std::begin(string);
  matcher.match([&begin](const std::string& pattern, decltype(begin) iterator)
  {
    std::cout << "Matched \"" << pattern << "\" on position "
      << std::distance(begin, iterator) << std::endl;
      return false;
  }, std::begin(string), std::end(string));
  
  std::cout << std::endl;
  
  // search can be performed on type different from pattern type
  std::vector<int> not_a_string;
  std::copy(std::begin(string), std::end(string), std::back_inserter(not_a_string));
  auto begin2 = std::begin(not_a_string);
  matcher.match([&begin2](const std::string& pattern, decltype(begin2) iterator)
  {
    std::cout << "Matched \"" << pattern << "\" on position "
      << std::distance(begin2, iterator) << std::endl;
      return false;
  }, std::begin(not_a_string), std::end(not_a_string));
}

