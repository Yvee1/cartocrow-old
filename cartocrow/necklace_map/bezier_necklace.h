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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 25-02-2020
*/

#ifndef CARTOCROW_NECKLACE_MAP_BEZIER_NECKLACE_H
#define CARTOCROW_NECKLACE_MAP_BEZIER_NECKLACE_H

#include <cartocrow/common/bezier_spline.h>
#include <cartocrow/common/core_types.h>

#include "cartocrow/necklace_map/necklace_shape.h"

namespace cartocrow {
namespace necklace_map {

class BezierNecklace : public NecklaceShape {
  public:
	using Ptr = std::shared_ptr<BezierNecklace>;

	static constexpr const Number kDistanceRatioEpsilon = 1.001;

	BezierNecklace(const BezierSpline spline, const Point& kernel);

	const Point& kernel() const override;

	const BezierSpline& spline() const;

	bool IsValid() const override;

	bool IntersectRay(const Number& angle_rad, Point& intersection) const override;

	Box ComputeBoundingBox() const override;

	Number ComputeCoveringRadiusRad(const Range::Ptr& range, const Number& radius) const override;

	Number ComputeDistanceToKernel(const Range::Ptr& range) const override;

	Number ComputeAngleAtDistanceRad(const Number& angle_rad, const Number& distance) const override;

	void Accept(NecklaceShapeVisitor& visitor) override;

  private:
	BezierSpline::CurveSet::const_iterator FindCurveContainingAngle(const Number& angle_rad) const;

	bool IntersectRay(const Number& angle_rad,
	                  const BezierSpline::CurveSet::const_iterator& curve_iter, Point& intersection,
	                  Number& t) const;

	bool ComputeAngleAtDistanceRad(const Point& point, const Number& distance,
	                               const BezierSpline::CurveSet::const_iterator& curve_point,
	                               const Number& t_point, Number& angle_rad) const;

	Number SearchCurveForAngleAtDistanceRad(const Point& point, const BezierCurve& curve,
	                                        const Number& squared_distance,
	                                        const CGAL::Orientation& orientation,
	                                        const Number& t_start) const;

	BezierSpline spline_;

	Point kernel_;
}; // class BezierNecklace

} // namespace necklace_map
} // namespace cartocrow

#endif //CARTOCROW_NECKLACE_MAP_BEZIER_NECKLACE_H