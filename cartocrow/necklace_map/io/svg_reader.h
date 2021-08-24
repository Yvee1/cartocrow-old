/*
The Necklace Map library implements the algorithmic
geo-visualization method by the same name, developed by
Bettina Speckmann and Kevin Verbeek at TU Eindhoven
(DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
Copyright (C) 2021  Netherlands eScience Center and TU Eindhoven

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

Created by tvl (t.vanlankveld@esciencecenter.nl) on 26-11-2019
*/

#ifndef CARTOCROW_NECKLACE_MAP_IO_SVG_READER_H
#define CARTOCROW_NECKLACE_MAP_IO_SVG_READER_H

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include <gflags/gflags.h>

#include "cartocrow/necklace_map/map_element.h"
#include "cartocrow/necklace_map/necklace.h"

namespace cartocrow {
namespace necklace_map {

class SvgReader {
  public:
	SvgReader();

	bool ReadFile(const std::filesystem::path& filename,
	              std::vector<necklace_map::MapElement::Ptr>& elements,
	              std::vector<necklace_map::Necklace::Ptr>& necklaces, int max_retries = 2);

	bool ReadFile(const std::filesystem::path& filename,
	              std::vector<necklace_map::MapElement::Ptr>& elements,
	              std::vector<necklace_map::Necklace::Ptr>& necklaces, Number& scale_factor,
	              int max_retries = 2);

	bool Parse(const std::string& input, std::vector<necklace_map::MapElement::Ptr>& elements,
	           std::vector<necklace_map::Necklace::Ptr>& necklaces, Number& scale_factor);
}; // class SvgReader

} // namespace necklace_map
} // namespace cartocrow

#endif //CARTOCROW_NECKLACE_MAP_IO_SVG_READER_H
