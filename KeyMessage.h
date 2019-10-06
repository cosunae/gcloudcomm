#pragma once
#include <cstddef>
#include <string>

struct KeyMessage {
  char key[8];
  int npatches;
  int myrank;
  size_t ilon_start, jlat_start;
  float dlon, dlat;
  size_t lonlen, latlen, levlen;
  size_t totlonlen, totlatlen;
};
