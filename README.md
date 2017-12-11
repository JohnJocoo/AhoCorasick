# AhoCorasick
Templated c++ implementation of A.V.Aho and M.Corasick efficient string matching algorithm.

# Usage 
There is an example in example.cpp file. Furthermore here is a step by step guide.

First you create instance of AhoCorasick template class where template parameter is type of patterns that will be used. 

    AhoCorasick<std::string> automation;
    
Then you add patterns to search for.

    automation.addPattern("quick");
    automation.addPattern("fox");
    
After you get matcher object that is able to perform actual search.

    auto matcher = automation.matcher();
    
Now you should call matcher.match() to search for patterns in data.

    std::string string{"quick red fox jumps over lazy brown dog"};
    auto begin = std::begin(string);
    matcher.match([&begin](const std::string& pattern, 
                           decltype(begin) iterator)
    {
      std::cout << "Matched \"" << pattern << "\" on position "
        << std::distance(begin, iterator) << std::endl;
      return false;
    }, std::begin(string), std::end(string));
    
Function match() receives callback function and iterators to data's begin and end, match() will search for patterns using Aho & Corasick state machine and call your callback on every pattern matched. Callback itself must receive 2 arguments:

1. Pattern that triggered match
2. Iterator to place in data where match happened (start of matched data).

Also callback's return type should be bool (or something convertible to bool). It must return false to continue search or true to stop and quit from match() function.

As some more advanced note - data is not required to be the same type as pattern. It just must be iterable and its values should be comparable with pattern values.

    std::vector<int> not_a_string;
    std::copy(std::begin(string), std::end(string), std::back_inserter(not_a_string));
    auto begin2 = std::begin(not_a_string);
    matcher.match([&begin2](const std::string& pattern, 
                            decltype(begin2) iterator)
    {
      std::cout << "Matched \"" << pattern << "\" on position "
        << std::distance(begin2, iterator) << std::endl;
      return false;
    }, std::begin(not_a_string), std::end(not_a_string));

