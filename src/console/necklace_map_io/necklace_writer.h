/*
The Necklace Map console application implements the algorithmic
geo-visualization method by the same name, developed by
Bettina Speckmann and Kevin Verbeek at TU Eindhoven
(DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
Copyright (C) 2019  Netherlands eScience Center and TU Eindhoven

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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 29-01-2020
*/

#ifndef CONSOLE_NECKLACE_MAP_IO_NECKLACE_WRITER_H
#define CONSOLE_NECKLACE_MAP_IO_NECKLACE_WRITER_H

#include <memory>
#include <ostream>
#include <vector>

#include "geoviz/common/core_types.h"
#include "geoviz/necklace_map/map_element.h"
#include "geoviz/necklace_map/necklace.h"
#include "console/necklace_map_io/detail/necklace_writer.h"


namespace geoviz
{

class NecklaceWriter
{
 public:
  using MapElement = necklace_map::MapElement;
  using Necklace = necklace_map::Necklace;

  NecklaceWriter();

  void Write
  (
    const std::vector<MapElement::Ptr>& elements,
    const std::vector<Necklace::Ptr>& necklaces,
    const Number& scale_factor,
    const WriterOptions::Ptr& options,
    std::ostream& out
  ) const;
}; // class NecklaceWriter

} // namespace geoviz

#endif //CONSOLE_NECKLACE_MAP_IO_NECKLACE_WRITER_H
