/*
The Necklace Map library implements the algorithmic geo-visualization
method by the same name, developed by Bettina Speckmann and Kevin Verbeek
at TU Eindhoven (DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 28-11-2019
*/

#ifndef GEOVIZ_NECKLACE_MAP_MAP_ELEMENT_H
#define GEOVIZ_NECKLACE_MAP_MAP_ELEMENT_H

#include <memory>
#include <unordered_map>

#include "geoviz/common/core_types.h"
#include "geoviz/common/region.h"
#include "geoviz/necklace_map/bead.h"
#include "geoviz/necklace_map/necklace.h"


namespace geoviz
{
namespace necklace_map
{

struct MapElement
{
  using Ptr = std::shared_ptr<MapElement>;
  using BeadMap = std::unordered_map<Necklace::Ptr, Bead::Ptr>;

  explicit MapElement(const std::string& id);
  explicit MapElement(const Region& region);

  //@param strict whether the value associated with each region must be strictly larger than 0.
  bool IsValid(const bool strict = true) const;

  Region region;
  Number value;  // Note that the value is correlated with the area of the bead, so its squared radius.

  BeadMap beads;
}; // struct MapElement

} // namespace necklace_map
} // namespace geoviz

#endif //GEOVIZ_NECKLACE_MAP_MAP_ELEMENT_H
