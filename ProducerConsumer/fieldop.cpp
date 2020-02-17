#include "DistributedField.h"
#include <array>
#include <iostream>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <string>

using namespace pybind11::literals;

PYBIND11_MODULE(fieldop, m) {
  pybind11::class_<DomainConf>(m, "DomainConf")
      .def(pybind11::init<int, int, int>())
      .def_readwrite("isize", &DomainConf::isize)
      .def_readwrite("jsize", &DomainConf::jsize)
      .def_readwrite("levels", &DomainConf::levels);

  pybind11::class_<BBox>(m, "BBox").def_readwrite("limits_", &BBox::limits_);

  pybind11::class_<DistributedField>(m, "DistributedField",
                                     pybind11::buffer_protocol())
      .def(pybind11::init<const std::string &, const DomainConf &,
                          unsigned long int>())
      .def("insertPatch", (void (DistributedField::*)(SinglePatch &)) &
                              DistributedField::insertPatch)
      .def("gatherField", &DistributedField::gatherField)
      .def("bboxPatches", &DistributedField::bboxPatches);

  pybind11::class_<field2d>(m, "field2d", pybind11::buffer_protocol())
      /// Construct from a buffer
      .def(pybind11::init([](pybind11::buffer const b) {
        pybind11::buffer_info info = b.request();
        if (info.format != pybind11::format_descriptor<float>::format() ||
            info.ndim != 2)
          throw std::runtime_error("Incompatible buffer format!");

        return field2d(info.shape[0], info.shape[1],
                       std::array<size_t, 2>{info.strides[0] / sizeof(float),
                                             info.strides[1] / sizeof(float)},
                       (float *)info.ptr);
      }))
      .def("__getitem__",
           [](const field2d &m, std::pair<ssize_t, ssize_t> i) {
             if (i.first >= m.isize() || i.second >= m.jsize())
               throw pybind11::index_error();
             return m(i.first, i.second);
           })
      .def_buffer([](field2d &m) -> pybind11::buffer_info {
        return pybind11::buffer_info(
            m.data(), /*Pointer to buffer*/ sizeof(float),
            /*Size of one scalar*/ pybind11::format_descriptor<float>::format(),
            /*Python struct-style format˓→descriptor*/ 2,
            /*Number of dimensions*/ {m.isize(), m.jsize()}, /*Buffer
                                                                dimensions*/
            {m.strides()[0] * sizeof(float), m.strides()[1] * sizeof(float)});
      });

  pybind11::class_<field3d>(m, "field3d", pybind11::buffer_protocol())
      /// Construct from a buffer
      .def(pybind11::init([](pybind11::buffer const b) {
        pybind11::buffer_info info = b.request();
        if (info.format != pybind11::format_descriptor<float>::format() ||
            info.ndim != 3)
          throw std::runtime_error("Incompatible buffer format!");

        return field3d(info.shape[0], info.shape[1], info.shape[2],
                       std::array<size_t, 3>{info.strides[0] / sizeof(float),
                                             info.strides[1] / sizeof(float),
                                             info.strides[2] / sizeof(float)},
                       (float *)info.ptr);
      }))
      .def(
          pybind11::init([](const int sizei, const int sizej, const int sizek) {
            return field3d(sizei, sizej, sizek);
          }))
      .def(pybind11::init([](const BBox box) { return field3d(box); }))
      .def("__getitem__",
           [](field3d &m, std::array<ssize_t, 3> pos) {
             if (pos[0] >= m.isize() || pos[1] >= m.jsize() ||
                 pos[2] >= m.ksize())
               throw pybind11::index_error();
             return m(pos[0], pos[1], pos[2]);
           })
      .def("__setitem__",
           [](field3d &m, std::array<ssize_t, 3> pos, float val) {
             if (pos[0] >= m.isize() || pos[1] >= m.jsize() ||
                 pos[2] >= m.ksize())
               throw pybind11::index_error();
             m(pos[0], pos[1], pos[2]) = val;
           })
      .def("ksize", &field3d::ksize)
      .def("jsize", &field3d::jsize)
      .def("isize", &field3d::isize)
      .def_buffer([](field3d &m) -> pybind11::buffer_info {
        return pybind11::buffer_info(
            m.data(), /*Pointer to buffer*/ sizeof(float),
            /*Size of one scalar*/ pybind11::format_descriptor<float>::format(),
            /*Python struct-style format˓→descriptor*/ 3,
            /*Number of dimensions*/ {m.isize(), m.jsize(), m.ksize()}, /*Buffer
                                                              dimensions*/
            {m.strides()[0] * sizeof(float), m.strides()[1] * sizeof(float),
             m.strides()[2] * sizeof(float)});
      });

  pybind11::class_<SinglePatch>(m, "SinglePatch", pybind11::buffer_protocol())
      /// Construct from a buffer
      .def(pybind11::init([](size_t ilonstart, size_t jlatstart, size_t lonlen,
                             size_t latlen, size_t lev,
                             pybind11::buffer const b) {
             pybind11::buffer_info info = b.request();
             if (info.format != pybind11::format_descriptor<float>::format() ||
                 info.ndim != 2)
               throw std::runtime_error("Incompatible buffer format!");

             return SinglePatch(ilonstart, jlatstart, lonlen, latlen, lev,
                                (float *)info.ptr);
           }),
           "ilonstart"_a, "jlatstart"_a, "lonlen"_a, "latlen"_a, "lev"_a,
           "ptr"_a)
      .def("ilonstart", &SinglePatch::ilonStart)
      .def("jlatstart", &SinglePatch::jlatStart)
      .def("lonlen", &SinglePatch::lonlen)
      .def("latlen", &SinglePatch::latlen)
      .def("lev", &SinglePatch::lev)
      .def("__getitem__",
           [](const SinglePatch &m, std::pair<ssize_t, ssize_t> i) {
             if (i.first >= m.isize() || i.second >= m.jsize())
               throw pybind11::index_error();
             return m(i.first, i.second);
           })
      .def_buffer([](SinglePatch &m) -> pybind11::buffer_info {
        return pybind11::buffer_info(
            m.data(), /*Pointer to buffer*/ sizeof(float),
            /*Size of one scalar*/ pybind11::format_descriptor<float>::format(),
            /*Python struct-style format˓→descriptor*/ 2,
            /*Number of dimensions*/ {m.isize(), m.jsize()}, /*Buffer
                                                              dimensions*/
            {m.strides()[0] * sizeof(float), m.strides()[1] * sizeof(float)});
      });
}
