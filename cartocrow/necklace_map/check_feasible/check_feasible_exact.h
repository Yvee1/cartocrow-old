/*
The Necklace Map library implements the algorithmic geo-visualization
method by the same name, developed by Bettina Speckmann and Kevin Verbeek
at TU Eindhoven (DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 06-05-2020
*/

#ifndef CARTOCROW_NECKLACE_MAP_DETAIL_CHECK_FEASIBLE_EXACT_H
#define CARTOCROW_NECKLACE_MAP_DETAIL_CHECK_FEASIBLE_EXACT_H

#include <map>

#include "check_feasible.h"

namespace cartocrow::necklace_map {
namespace detail {

// The exact algorithm for the feasibility decision problem computes all possible node orderings until it finds a valid placement.
// This takes O(n*log(n) + n^2K4^K) time, where n is the number of nodes, and K is the 'width' of the node set (i.e. the maximum number of valid intervals intersected by a ray originating for the necklace kernel).
class CheckFeasibleExact : public CheckFeasible {
  public:
	CheckFeasibleExact(NodeSet& nodes);

	bool operator()() override;

  private:
	void SplitCircle(const TaskSlice& first_slice, const BitString& layer_set);

	bool FeasibleFromSlice(const size_t first_slice_index, const BitString& first_slice_layer_set);

	void AssignAngle(const Number<Inexact>& angle_rad, std::shared_ptr<Bead>& bead) override;

	using BeadAngleMap = std::map<Number<Inexact>, std::shared_ptr<Bead>>;
	BeadAngleMap bead_angles_;
}; // class CheckFeasibleExact

} // namespace detail
} // namespace cartocrow::necklace_map

#endif //CARTOCROW_NECKLACE_MAP_DETAIL_CHECK_FEASIBLE_EXACT_H
