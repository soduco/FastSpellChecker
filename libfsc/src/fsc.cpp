#include <fsc.hpp>

#include <cstdint>
#include <cstring>
#include <climits>
#include <deque>
#include <vector>
#include <unordered_map>
#include <cassert>

#include <iostream>

struct Dictionary::DictionaryImplBase
{
  virtual ~DictionaryImplBase() = default;

  virtual void              load(std::string_view word_list[], std::size_t n)       = 0;
  virtual bool              has_matches(std::string_view word, int d) const         = 0;
  virtual DictionaryMatch   best_match(std::string_view word, int d) const          = 0;
};

namespace
{

  template <class To, class From>
  To bit_cast(const From& src) noexcept
  {
    return *(To*)(&src);
  }

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
    static constexpr int PTR_MASK = 0x00FFFFFF;
    static constexpr int DATA_MASK = 0xFF000000;
    static constexpr int DATA_SHIFT = 48;

    match_info_t() = default;

    match_info_t(const char* original_word, int distance, int deletion_pos = -1)
    {
      this->m_distance = distance;
      this->m_pos_del = deletion_pos;
      this->m_ptr = (uintptr_t) original_word;
    }

    void set(const char* original_word, int distance, int deletion_pos = -1)
    {
      this->m_distance = distance;
      this->m_pos_del = deletion_pos;
      this->m_ptr = (uintptr_t) original_word;
    }


    const char*  get_word() const { return (const char*)(uintptr_t)(m_ptr); }
    int          get_deletion_position() const { return m_pos_del; }
    int          get_distance() const { return m_distance; }
    void         set_word(const char* word) { m_ptr = (uintptr_t)word; }
    void         set_deletion_position(int pos)  { m_pos_del = pos; }
    void         set_distance(int d)  { m_distance = d; }

    friend std::ostream& operator<<(std::ostream& os, match_info_t x)
    {
      os << "(" << x.get_word() << ", d=" << x.get_distance() << ", p=" << x.get_deletion_position() << ")";
      return os;
    }

  private:
    int            m_distance : 4 = 0;
    int            m_pos_del : 12 = -1;
    std::uintptr_t m_ptr : 48     = 0;
  };

  using matches_t = std::vector<match_info_t>;
  using dic_map_t = std::unordered_map<const char*, matches_t, string_hash, string_cmp>;

  struct DictionaryImplHashTable final : public Dictionary::DictionaryImplBase
  {
    static constexpr int kMaxDist = 2;

    void            load(std::string_view word_list[], std::size_t n) final;
    bool            has_matches(std::string_view word, int d) const final;
    DictionaryMatch best_match(std::string_view word, int d) const final;

  private:
    void        add_word(char buffer[], int len, match_info_t from, int max_dist);
    void        get_best_match(char buffer[], int len, int delpos, int current_score, int max_score,
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
    char buffer[256];
    for (std::size_t i = 0; i < n; ++i)
    {
      auto len = word_list[i].size();
      assert(len < sizeof(buffer)); // FIXME error instead

      std::memcpy(buffer, word_list[i].data(), len);
      buffer[len] = 0;
      this->add_word(buffer, len, match_info_t{}, kMaxDist);
    }

    /* Debug dict
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

  void DictionaryImplHashTable::add_word(char buffer[], int len, match_info_t from, int max_dist)
  {
    assert(from.get_distance() <= max_dist);
    const char* current_word = this->insert(buffer, from);
    int         current_distance = from.get_distance();


    if (current_distance >= max_dist)
      return;


    if (from.get_word() == nullptr)
      from.set_word(current_word);
    from.set_distance(from.get_distance() + 1);

    // Add all deletions of distance +1
    for (int i = 0; i < len; ++i)
    {
      // Only remove caracters after le last removal
      if (i < from.get_deletion_position())
        continue;

      char c = buffer[i];
      from.set_deletion_position(i);
      std::memmove(buffer + i, buffer + i + 1, len - i); // Mind the trailing NULL caracter
      this->add_word(buffer, len - 1, from, max_dist);
      std::memmove(buffer + i + 1, buffer + i, len - i);
      buffer[i] = c;
    }
  }

  void DictionaryImplHashTable::get_best_match(char buffer[], int len, int delpos, int current_score, int max_score, DictionaryMatch& best_match, bool stop_first_found) const
  {
    assert(current_score <= best_match.distance);
    assert(current_score <= max_score);

    if (stop_first_found && best_match.distance <= max_score)
      return;

    auto r = m_dic.find(buffer);
    bool found = r != m_dic.end();

    if (found)
    {
      for (auto m : r->second)
      {
        int s = m.get_distance() + current_score;

        // Del + Ins in the same place == Substitution
        if (delpos >= 0 && m.get_deletion_position() == delpos)
          s -= 1;

        if (s > max_score)
          continue;

        if (s < best_match.distance)
        {
          best_match.distance = s;
          best_match.word = m.get_word();
          best_match.count = 1;
        }
        else if (s == best_match.distance)
        {
          best_match.count += 1;
        }

        if (m.get_distance() == 0) // Best match (no need to loop anymore)
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
    for (int i = 0; i < len; ++i)
    {
      // Only remove caracters after le last removal
      if (i < delpos)
        continue;

      char c = buffer[i];
      std::memmove(buffer + i, buffer + i + 1, len - i);
      get_best_match(buffer, len - 1, i, current_score + 1, max_score, best_match, stop_first_found);
      std::memmove(buffer + i + 1, buffer + i, len - i);
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
    char buffer[256];
    std::memcpy(buffer, word.data(), n);
    buffer[n] = 0;

    this->get_best_match(buffer, n, -1, 0, d, best_match, true);
    assert((best_match.distance == INT_MAX) == (best_match.word == nullptr));

    return best_match.distance <= d;
  }


  DictionaryMatch DictionaryImplHashTable::best_match(std::string_view word, int d) const
  {
    DictionaryMatch best_match;
    best_match.distance = INT_MAX;
    best_match.count = 0;
    best_match.word = nullptr;

    auto n = word.size();
    char buffer[256];
    std::memcpy(buffer, word.data(), n);
    buffer[n] = 0;

    this->get_best_match(buffer, n, -1, 0, d, best_match, false);
    assert((best_match.distance == INT_MAX) == (best_match.word == nullptr));

    return best_match;
  }

} // namespace

Dictionary::Dictionary()
{
  m_impl = std::make_unique<DictionaryImplHashTable>();
}


Dictionary::~Dictionary()
{
}


void Dictionary::load(std::string_view word_list[], std::size_t n)
{
  m_impl->load(word_list, n);
}


bool Dictionary::has_matches(std::string_view word, int d)
{
  return m_impl->has_matches(word, d);
}


DictionaryMatch Dictionary::best_match(std::string_view word, int d)
{
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

