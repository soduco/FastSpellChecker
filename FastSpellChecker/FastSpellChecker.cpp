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
    /*
    auto sz = py::len(word_list);

    std::vector<std::string_view> wl;
    wl.reserve(sz);

    for (auto w : word_list)
    {
      if (!py::isinstance<py::str>(w))
        throw py::type_error("A list of 'str' was expected.")

      PyObject*   o   = w.ptr();
      const char* ptr = PyUnicode_AS_DATA(o);
      std::size_t
    */
  }

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


PYBIND11_MODULE(fastspellchecker, m) {
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
    .def("best_match", &CPPDictionary::best_match);

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}
