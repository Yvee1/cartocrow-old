/*
The CartoCrow library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 05-12-2019
*/

#ifndef CARTOCROW_COMMON_POLYGON_H
#define CARTOCROW_COMMON_POLYGON_H

#include <CGAL/Polygon_with_holes_2.h>

#include "cartocrow/common/cgal_types.h"

namespace cartocrow {

using Polygon = CGAL::Polygon_2<Kernel>;
using Polygon_with_holes = CGAL::Polygon_with_holes_2<Kernel>;

class ComputeCentroid {
  public:
	Point operator()(const Polygon& shape) const;
	Point operator()(const Polygon_with_holes& shape) const;
}; // class ComputeCentroid

} // namespace cartocrow

#endif //CARTOCROW_COMMON_POLYGON_H
