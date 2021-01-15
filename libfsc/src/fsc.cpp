#include <fsc.hpp>

#include <cstdint>
#include <cstring>
#include <climits>
#include <deque>
#include <vector>
#include <unordered_map>
#include <cassert>

#include <iostream>

namespace
{
  constexpr int        kMaxWordLength = 255;
  static constexpr int kMaxDist = 2;
};


struct Dictionary::DictionaryImplBase
{
  virtual ~DictionaryImplBase() = default;

  virtual void              load(std::string_view word_list[], std::size_t n)       = 0;
  virtual bool              has_matches(std::string_view word, int d) const         = 0;
  virtual DictionaryMatch   best_match(std::string_view word, int d) const          = 0;
};

namespace
{
  struct DictionaryErrCategory : std::error_category
  {
    const char* name() const noexcept override;
    std::string message(int ev) const override;
  };

  struct string_hash
  {
    using hash_type      = std::hash<std::string_view>;
    using is_transparent = void;

    size_t operator()(const char* str) const { return hash_type{}(str); }
    size_t operator()(std::string_view str) const { return hash_type{}(str); }
    size_t operator()(std::string const& str) const { return hash_type{}(str); }
  };

  struct string_cmp
  {
    bool operator()(const char* a, const char* b) const { return strcmp(a, b) == 0; }
  };

  struct match_info_t
  {
    match_info_t()
    {
      this->m_distance         = 0;
      this->m_ptr              = 0;
      std::memset(this->m_pos_deletions, -1, kMaxDist + 1);
    }


    const char*   get_word() const { return (const char*)(uintptr_t)(m_ptr); }
    const int8_t* get_deletion_positions() const { return m_pos_deletions; }
    int           get_last_deletion_position() const { return m_pos_deletions[m_distance]; }
    int           get_distance() const { return m_distance; }
    void          set_word(const char* word) { m_ptr = (uintptr_t)word; }
    void          set_deletion_position(int d, int value) { m_pos_deletions[d] = value; }
    void          set_distance(int d) { m_distance = d; }

    friend std::ostream& operator<<(std::ostream& os, match_info_t x)
    {
      os << "(" << x.get_word() << ", d=" << x.get_distance() << ", p=";
      for (int i = 0; i < kMaxDist + 1; ++i)
        os << (int)x.m_pos_deletions[i] << ',';
      os << ")";
      return os;
    }

  private:
    struct
    {
      int            m_distance : 4;
      int            m_pos_del : 12;
      std::uintptr_t m_ptr : 48;
    };
    int8_t           m_pos_deletions[kMaxDist + 1];
  };

  using matches_t = std::vector<match_info_t>;
  using dic_map_t = std::unordered_map<const char*, matches_t, string_hash, string_cmp>;

  struct DictionaryImplHashTable final : public Dictionary::DictionaryImplBase
  {
    void            load(std::string_view word_list[], std::size_t n) final;
    bool            has_matches(std::string_view word, int d) const final;
    DictionaryMatch best_match(std::string_view word, int d) const final;

  private:
    void add_word(char buffer[], int len, int subtr_start, match_info_t from, int max_dist);
    void get_best_match(char buffer[], int len, int subtr_start, int8_t delpos[], int current_score, int max_score,
                        DictionaryMatch& best_match, bool stop_first_found) const;
    const char* insert(const char* new_word, match_info_t from);

    dic_map_t m_dic;
    std::deque<std::string> m_words;
  };

  const char* DictionaryImplHashTable::insert(const char* new_word, match_info_t from)
  {
    const char* key;
    matches_t*  matches;

    if (auto r = m_dic.find(new_word); r != m_dic.end())
    {
      key = r->first;
      matches = &r->second;
    }
    else
    {
      m_words.push_back(new_word);
      key     = m_words.back().c_str();
      matches = &m_dic[key];
    }

    if (from.get_word() == nullptr)
      from.set_word(key);

    matches->push_back(from);
    return key;
  }

  void DictionaryImplHashTable::load(std::string_view word_list[], std::size_t n)
  {
    char buffer[kMaxWordLength + 1];
    for (std::size_t i = 0; i < n; ++i)
    {
      auto len = word_list[i].size();
      if (len >= kMaxWordLength)
        throw std::runtime_error("Word exceeds max length (255)");

      std::memcpy(buffer, word_list[i].data(), len);
      buffer[len] = 0;
      this->add_word(buffer, len, 0, match_info_t{}, kMaxDist);
    }

    // Debug dict
    /*
        for (auto s : m_words)
          std::cout << s << "\n";

        for (auto&& [k, v] : m_dic)
        {
          std::cout << k << " : ";
          for (auto m : v)
            std::cout << m << " " ;
          std::cout << "\n";
        }
        std::cout << std::endl;
    */
  }

  void DictionaryImplHashTable::add_word(char buffer[], int len, int subtr_start, match_info_t from, int max_dist)
  {
    assert(from.get_distance() <= max_dist);
    const char* current_word = this->insert(buffer, from);
    int         current_distance = from.get_distance();


    if (current_distance >= max_dist)
      return;


    if (from.get_word() == nullptr)
      from.set_word(current_word);
    from.set_distance(current_distance + 1);

    // Add all deletions of distance +1
    // Only remove caracters after le last removal
    for (int i = subtr_start; i < len; ++i)
    {
      char c = buffer[i];
      from.set_deletion_position(current_distance, i + current_distance);
      from.set_deletion_position(current_distance + 1, -1);
      std::memmove(buffer + 1, buffer, i);
      this->add_word(buffer + 1, len - 1, i, from, max_dist);
      std::memmove(buffer, buffer + 1, i);
      buffer[i] = c;
    }
  }



  namespace
  {


    int levenshtein_of(const int8_t a_deletion_pos[], const int8_t b_deletion_pos[])
    {
      int a = 0;
      int b = 0;
      while (a_deletion_pos[a] == a)
        a++;
      while (b_deletion_pos[b] == b)
        b++;


      // a_deletion_pos and b_deletion_pos are sorted
      int i = a;
      int j = b;
      int subst = std::min(a, b);
      while (a_deletion_pos[i] != -1 and b_deletion_pos[j] != -1)
      {
        int del_pos_a = (a_deletion_pos[i] - a);
        int del_pos_b = (b_deletion_pos[i] - b);
        if( del_pos_a == del_pos_b)
        {
          subst++;
          i++; j++;
        }
        else if (del_pos_a < del_pos_b)
        {
          i++;
        }
        else
        {
          j++;
        }
      }
      while (a_deletion_pos[i] != -1)
        i++;
      while (b_deletion_pos[j] != -1)
        j++;

      return i + j - subst;
    }
  }

  void DictionaryImplHashTable::get_best_match(char buffer[], int len, int substr_start, int8_t del_pos[], int current_score, int max_score, DictionaryMatch& best_match, bool stop_first_found) const
  {
    assert(current_score <= best_match.distance);
    assert(current_score <= max_score);

    if (stop_first_found && best_match.distance <= max_score)
      return;


    auto r        = m_dic.find(buffer);
    bool found    = r != m_dic.end();

    if (found)
    {
      del_pos[current_score] = -1;
      for (auto m : r->second)
      {
        int s;

        // Exact match (only deletion required)
        if (m.get_distance() == 0)
        {
          s = current_score;
        }
        else
        {
          // Possible substitution instead of indels
          s = levenshtein_of(del_pos, m.get_deletion_positions());
          /*
          std::cout << '[';
          for (int i = 0; i <= current_score; ++i)
            std::cout << (int)del_pos[i] << ",";
          std::cout << "] vs " << m << "\n";
          std::cout << "(" << buffer << "," << m.get_word() << ") = " << s << "\n";
          */
        }

        if (s < best_match.distance)
        {
          best_match.distance = s;
          best_match.word     = m.get_word();
          best_match.count    = 1;
          //std::cout << "Best setted (" << buffer << "," << m.get_word() << ") = " << s << "\n";
        }
        else if (s == best_match.distance)
        {
          best_match.count += 1;
        }

        // If it is exact, we cannot do better
        if (m.get_distance() == 0)
          break;
      }
    }

    // Avoid useless computations that would not improve the score
    if ((current_score + 1) >= best_match.distance || (current_score + 1) > max_score)
      return;

    // If find-only
    if (stop_first_found && best_match.distance <= max_score)
      return;

    // Try suppressions
    // Only remove caracters after le last removal
    for (int i = substr_start; i < len; ++i)
    {
      char c = buffer[i];
      del_pos[current_score] = i + current_score;
      del_pos[current_score + 1] = -1;

      std::memmove(buffer + 1, buffer, i);
      get_best_match(buffer + 1, len - 1, i, del_pos, current_score + 1, max_score, best_match, stop_first_found);
      std::memmove(buffer, buffer + 1, i);

      buffer[i] = c;
    }
  }

  bool DictionaryImplHashTable::has_matches(std::string_view word, int d) const
  {
    DictionaryMatch best_match;
    best_match.distance = INT_MAX;
    best_match.count = 0;
    best_match.word = nullptr;



    auto n = word.size();
    char    buffer[256];
    int8_t  del_pos[256] = {-1};

    std::memcpy(buffer, word.data(), n);
    buffer[n] = 0;

    this->get_best_match(buffer, n, 0, del_pos, 0, d, best_match, true);
    assert((best_match.distance == INT_MAX) == (best_match.word == nullptr));

    return best_match.distance <= d;
  }


  DictionaryMatch DictionaryImplHashTable::best_match(std::string_view word, int d) const
  {
    DictionaryMatch best_match;
    best_match.distance = INT_MAX;
    best_match.count = 0;
    best_match.word = nullptr;

    auto    n = word.size();
    char    buffer[256];
    int8_t  del_pos[256] = {-1};
    std::memcpy(buffer, word.data(), n);
    buffer[n] = 0;

    this->get_best_match(buffer, n, 0, del_pos, 0, d, best_match, false);
    assert((best_match.distance == INT_MAX) == (best_match.word == nullptr));

    return best_match;
  }

} // namespace

Dictionary::Dictionary()
{
}


Dictionary::~Dictionary()
{
}


void Dictionary::load(std::string_view word_list[], std::size_t n)
{
  m_impl = std::make_unique<DictionaryImplHashTable>();
  m_impl->load(word_list, n);
}

namespace
{
  void check_params(std::string_view word, int d)
  {
    if (d > kMaxDist)
      throw std::runtime_error("Invalid distance (Must be <= 2)");

    if (word.size() > kMaxWordLength)
      throw std::runtime_error("Word too long (should be <= 255)");
  }
}


bool Dictionary::has_matches(std::string_view word, int d)
{
  check_params(word, d);
  return m_impl->has_matches(word, d);
}


DictionaryMatch Dictionary::best_match(std::string_view word, int d)
{
  check_params(word, d);
  return m_impl->best_match(word, d);
}




DictionaryMatch::operator bool() const
{
  return distance >= 0;
}

std::ostream& operator<<(std::ostream& os, const DictionaryMatch& m)
{
  os << "(" << m.word << ", d=" << m.distance << ", c=" << m.count << ")";
  return os;
}

