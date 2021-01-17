#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "fsc.hpp"

namespace py = pybind11;


class CPPDictionary
{
public:
  void load(std::vector<std::string_view> word_list)
  {
    m_handle.load(word_list.data(), word_list.size());
  }

  void add_word(std::string_view word)
  {
    m_handle.add_word(word);
  }

  int max_word_length() const { return m_handle.max_word_length(); }

  bool has_matches(std::string_view word, int d) { return m_handle.has_matches(word, d); }

  py::object best_match(std::string_view word, int d)
  {
    auto r = m_handle.best_match(word, d);
    if (r.distance > d)
      return py::none();

    py::dict result;
    result["word"]     = py::str(r.word);
    result["distance"] = r.distance;
    result["count"]    = r.count;
    return result;
  }

private:
  Dictionary m_handle;
};


PYBIND11_MODULE(_backend, m) {
  m.doc() = R"docstring(
        Pybind11 example plugin
        -----------------------
        .. currentmodule:: libdictionary
        .. autosummary::
           :toctree: _generate
           CPPDictionary
           )docstring";


  py::class_<CPPDictionary>(m, "CPPDictionary")
    .def(py::init<>())
    .def("load", &CPPDictionary::load)
    .def("has_matches", &CPPDictionary::has_matches)
    .def("best_match", &CPPDictionary::best_match)
    .def("add_word", &CPPDictionary::add_word)
    .def("max_word_length", &CPPDictionary::max_word_length)
    ;

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}
