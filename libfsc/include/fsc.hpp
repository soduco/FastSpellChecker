#pragma once

#include <memory>
#include <string_view>
#include <iosfwd>


struct DictionaryMatch
{
  const char* word; // One a the best match
  int         distance;
  int         count;


  operator bool() const;
};

std::ostream& operator<< (std::ostream& os, const DictionaryMatch& m);



class Dictionary
{
public:
  Dictionary();
  ~Dictionary();

  void              load(std::string_view word_list[], std::size_t n);
  bool              has_matches(std::string_view word, int d);
  DictionaryMatch   best_match(std::string_view word, int d);


  struct DictionaryImplBase;
private:
  std::unique_ptr<DictionaryImplBase> m_impl;
};

