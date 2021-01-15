#include <fsc.hpp>

#include <gtest/gtest.h>
#include <string>

using namespace std::literals;

TEST(Dico, simple)
{
  Dictionary t;

  {
    std::string_view data[] = {
      "aaab",
      "rue du a",
      "abba",
      "aba",
      "bab",
    };
    t.load(data, sizeof(data) / sizeof(std::string_view));
  }

  {
    auto m = t.best_match("abab", 2);
    ASSERT_EQ(m.distance, 1) << "with abab";
    ASSERT_EQ(m.count, 3) << "with abab";
    ASSERT_TRUE(m.word == "aba"sv || m.word == "bab"sv || m.word == "aaab"sv);
  }

  {
    auto m = t.best_match("ab", 2);
    ASSERT_EQ(m.distance, 1)  << "with ab";
    ASSERT_EQ(m.count, 2) << "with ab";
    ASSERT_TRUE(m.word == "aba"sv || m.word == "bab"sv);
  }

  {
    std::string_view data[] = { "Saint Denis" };
    t.load(data, 1);

    auto m = t.best_match("saint denis", 2);
    ASSERT_EQ(m.distance, 2);
    ASSERT_EQ(m.count, 1);
    ASSERT_TRUE(m.word == "Saint Denis"sv);
  }
}








TEST(DICO, test_substitutions)
{
  Dictionary t;
  {
    std::string_view data[] = {"RUE DES MARGUETTES"};
    t.load(data, 1);
  };

  {
    auto m = t.best_match("RUE DEs MARGUATTES", 2);
    ASSERT_EQ(m.distance, 2);
    ASSERT_EQ(m.count, 1);
  }
}






extern std::string_view test_data[];
extern std::size_t test_data_size;

TEST(Dico, large_data)
{
  Dictionary t;
  t.load(test_data, test_data_size);

  {
    auto m = t.best_match("petites-ecuries", 2);
    ASSERT_EQ(m.distance, 1);
  }

  {
    ASSERT_FALSE(t.has_matches("petites-ecuries", 0));
    ASSERT_TRUE(t.has_matches("petites-ecuries", 1));
  }

  {
    ASSERT_FALSE(t.has_matches("s-ecuries", 0));
    ASSERT_FALSE(t.has_matches("s-ecuries", 2));
  }
}
