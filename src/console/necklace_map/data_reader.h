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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 03-12-2019
*/

#ifndef CONSOLE_NECKLACE_MAP_DATA_READER_H
#define CONSOLE_NECKLACE_MAP_DATA_READER_H

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "console/common/detail/table_parser.h"
#include "geoviz/necklace_map/map_element.h"


namespace geoviz
{

class DataReader : public detail::TableParser
{
 public:
  DataReader();

  bool ReadFile
  (
    const std::string& filename,
    const std::string& value_name,
    std::vector<necklace_map::MapElement::Ptr>& elements,
    int max_retries = 2
  );

  bool Parse
  (
    std::istream& in,
    const std::string& value_name,
    std::vector<necklace_map::MapElement::Ptr>& elements
  );
}; // class DataReader

} // namespace geoviz

#endif //CONSOLE_NECKLACE_MAP_DATA_READER_H