/*
The Flow Map library implements the algorithmic geo-visualization
method by the same name, developed by Kevin Verbeek, Kevin Buchin,
and Bettina Speckmann at TU Eindhoven
(DOI: 10.1007/s00453-013-9867-z & 10.1109/TVCG.2011.202).
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
*/

#include "spiral_tree_obstructed_algorithm.h"

#include <cmath>
#include <optional>
#include <ostream>

#include "../core/core.h"
#include "../necklace_map/circular_range.h"
#include "intersections.h"
#include "polar_segment.h"
#include "spiral_segment.h"
#include "sweep_circle.h"
#include "sweep_interval.h"

namespace cartocrow::flow_map {

SpiralTreeObstructedAlgorithm::Event::Event(PolarPoint position, Type type,
                                            SpiralTreeObstructedAlgorithm* alg)
    : m_position(position), m_type(type), m_alg(alg) {}

Number<Inexact> SpiralTreeObstructedAlgorithm::Event::r() const {
	return m_position.r();
}

Number<Inexact> SpiralTreeObstructedAlgorithm::Event::phi() const {
	return m_position.phi();
}

SpiralTreeObstructedAlgorithm::Event::Type SpiralTreeObstructedAlgorithm::Event::type() const {
	return m_type;
}

SpiralTreeObstructedAlgorithm::NodeEvent::NodeEvent(PolarPoint position,
                                                    SpiralTreeObstructedAlgorithm* alg)
    : Event(position, Type::NODE, alg) {}

void SpiralTreeObstructedAlgorithm::NodeEvent::handle() {
	// TODO
	std::cout << "> handling node event" << std::endl;
	m_alg->m_debugPainting->setMode(renderer::GeometryRenderer::stroke);
	m_alg->m_debugPainting->setStroke(Color{240, 120, 0}, 1);
	m_alg->m_debugPainting->draw(m_alg->m_tree->rootPosition() +
	                             (m_position.toCartesian() - CGAL::ORIGIN));
	m_alg->m_debugPainting->drawText(
	    m_alg->m_tree->rootPosition() + (m_position.toCartesian() - CGAL::ORIGIN), "node");
}

SpiralTreeObstructedAlgorithm::VertexEvent::VertexEvent(PolarPoint position,
                                                        std::shared_ptr<SweepEdge> e1,
                                                        std::shared_ptr<SweepEdge> e2,
                                                        SpiralTreeObstructedAlgorithm* alg)
    : Event(position, Type::VERTEX, alg), m_e1(e1), m_e2(e2), m_side(determineSide()) {}

void SpiralTreeObstructedAlgorithm::VertexEvent::handle() {
	using enum SweepInterval::Type;

	std::string side = "";
	switch (m_side) {
	case Side::LEFT:
		side = "left";
		break;
	case Side::RIGHT:
		side = "right";
		break;
	case Side::NEAR:
		side = "near";
		break;
	case Side::FAR:
		side = "far";
	}
	std::cout << "> handling " << side << " vertex event" << std::endl;
	m_alg->m_debugPainting->setMode(renderer::GeometryRenderer::stroke);
	m_alg->m_debugPainting->setStroke(Color{150, 150, 150}, 0.5);
	m_alg->m_debugPainting->draw(m_alg->m_tree->rootPosition() +
	                             (m_position.toCartesian() - CGAL::ORIGIN));
	m_alg->m_debugPainting->drawText(
	    m_alg->m_tree->rootPosition() + (m_position.toCartesian() - CGAL::ORIGIN), side);

	if (m_side == Side::LEFT) {
		handleLeft();
	} else if (m_side == Side::RIGHT) {
		handleRight();
	} else if (m_side == Side::NEAR) {
		handleNear();
	} else if (m_side == Side::FAR) {
		handleFar();
	}

	insertJoinEvents();
}

void SpiralTreeObstructedAlgorithm::VertexEvent::handleLeft() {
	using enum SweepInterval::Type;
	using enum SweepEdgeShape::Type;
	SweepInterval* outsideInterval = m_e2->nextInterval();
	if (outsideInterval->type() == SHADOW) {
		auto result = m_alg->m_circle.switchEdge(*m_e2, m_e1);
	} else if (outsideInterval->type() == REACHABLE) {
		auto spiral = std::make_shared<SweepEdge>(
		    SweepEdgeShape(RIGHT_SPIRAL, m_position, m_alg->m_tree->restrictingAngle()));
		if (spiral->shape().departsToLeftOf(m_e1->shape())) {
			auto result = m_alg->m_circle.splitFromEdge(*m_e2, m_e1, spiral);
			result.middleInterval->setType(SHADOW);
		} else {
			auto result = m_alg->m_circle.switchEdge(*m_e2, m_e1);
		}
	} else if (outsideInterval->type() == OBSTACLE) {
		// a vertex event cannot have an obstacle interval on the outside
		assert(false);
	}
}

void SpiralTreeObstructedAlgorithm::VertexEvent::handleRight() {
	using enum SweepInterval::Type;
	using enum SweepEdgeShape::Type;
	SweepInterval* outsideInterval = m_e1->previousInterval();
	if (outsideInterval->type() == SHADOW) {
		auto result = m_alg->m_circle.switchEdge(*m_e1, m_e2);
	} else if (outsideInterval->type() == REACHABLE) {
		auto spiral = std::make_shared<SweepEdge>(
		    SweepEdgeShape(LEFT_SPIRAL, m_position, m_alg->m_tree->restrictingAngle()));
		if (m_e2->shape().departsToLeftOf(spiral->shape())) {
			auto result = m_alg->m_circle.splitFromEdge(*m_e1, spiral, m_e2);
			result.middleInterval->setType(SHADOW);
		} else {
			auto result = m_alg->m_circle.switchEdge(*m_e1, m_e2);
		}
	} else if (outsideInterval->type() == OBSTACLE) {
		// a vertex event cannot have an obstacle interval on the outside
		assert(false);
	}
}

void SpiralTreeObstructedAlgorithm::VertexEvent::handleNear() {
	using enum SweepInterval::Type;
	using enum SweepEdgeShape::Type;

	SweepInterval* interval = m_alg->m_circle.intervalAt(phi());
	if (interval->type() == OBSTACLE) {
		// case 1: concave corner of an obstacle
		auto result = m_alg->m_circle.splitFromInterval(m_e1, m_e2);
		result.middleInterval->setType(SHADOW);

	} else if (interval->type() == SHADOW) {
		// case 2: convex corner of an obstacle, laying in the shadow
		auto result = m_alg->m_circle.splitFromInterval(m_e2, m_e1);
		result.middleInterval->setType(OBSTACLE);

	} else if (interval->type() == REACHABLE) {
		// case 3: convex corner of an obstacle, laying in reachable area
		auto leftSpiral = std::make_shared<SweepEdge>(
		    SweepEdgeShape(LEFT_SPIRAL, m_position, m_alg->m_tree->restrictingAngle()));
		auto rightSpiral = std::make_shared<SweepEdge>(
		    SweepEdgeShape(RIGHT_SPIRAL, m_position, m_alg->m_tree->restrictingAngle()));

		if (m_e2->shape().departsToLeftOf(leftSpiral->shape())) {
			// case 3a: vertex casts shadow to the right of the obstacle
			auto result = m_alg->m_circle.splitFromInterval(leftSpiral, m_e2, m_e1);
			result.middleLeftInterval->setType(OBSTACLE);
			result.middleRightInterval->setType(SHADOW);

		} else if (rightSpiral->shape().departsToLeftOf(m_e1->shape())) {
			// case 3b: vertex casts shadow to the left of the obstacle
			auto result = m_alg->m_circle.splitFromInterval(m_e2, m_e1, rightSpiral);
			result.middleLeftInterval->setType(SHADOW);
			result.middleRightInterval->setType(OBSTACLE);

		} else {
			// case 3c: no shadow
			auto result = m_alg->m_circle.splitFromInterval(m_e2, m_e1);
			result.middleInterval->setType(OBSTACLE);
		}
	}
}

void SpiralTreeObstructedAlgorithm::VertexEvent::handleFar() {
	using enum SweepInterval::Type;
	using enum SweepEdgeShape::Type;

	if (m_e1->nextInterval() == m_e2->previousInterval()) {
		// case 1: convex corner of an obstacle
		auto previousIntervalType = m_e1->previousInterval()->type();
		auto nextIntervalType = m_e2->nextInterval()->type();

		if (previousIntervalType == nextIntervalType) {
			// if both sides are SHADOW or both sides are REACHABLE, the entire
			// merged interval simply becomes that type, too
			auto result = m_alg->m_circle.mergeToInterval(*m_e1, *m_e2);
			result.mergedInterval->setType(previousIntervalType);
		} else {
			// else only one side is REACHABLE, we need to add a spiral to
			// separate them
			SweepEdgeShape::Type spiralType =
			    previousIntervalType == REACHABLE ? LEFT_SPIRAL : RIGHT_SPIRAL;
			auto spiral = std::make_shared<SweepEdge>(
			    SweepEdgeShape(spiralType, m_position, m_alg->m_tree->restrictingAngle()));
			auto result = m_alg->m_circle.mergeToEdge(*m_e1, *m_e2, spiral);
		}

	} else if (m_e2->nextInterval() == m_e1->previousInterval()) {
		// case 2: concave corner of an obstacle
		auto result = m_alg->m_circle.mergeToInterval(*m_e2, *m_e1);
		result.mergedInterval->setType(OBSTACLE);

	} else {
		assert(false);
	}
}

SpiralTreeObstructedAlgorithm::VertexEvent::Side
SpiralTreeObstructedAlgorithm::VertexEvent::determineSide() {
	if (m_e1->shape().nearR() == r() && m_e2->shape().nearR() == r()) {
		return Side::NEAR;
	} else if (m_e1->shape().farR() == r() && m_e2->shape().farR() == r()) {
		return Side::FAR;
	} else if (m_e1->shape().nearR() == r() && m_e2->shape().farR() == r()) {
		return Side::LEFT;
	} else if (m_e1->shape().farR() == r() && m_e2->shape().nearR() == r()) {
		return Side::RIGHT;
	}
	assert(false); // near or far r (of both m_e1 and m_e2) needs to be equal to r() of event
}

void SpiralTreeObstructedAlgorithm::VertexEvent::insertJoinEvents() {
	auto [begin, end] = m_alg->m_circle.edgesAt(phi());
	/*if (begin == end) {
		return;
	}*/
	if (begin != m_alg->m_circle.begin()) {
		insertJoinEventFor(std::prev(begin));
	}
	for (auto e = begin; e != end; ++e) {
		insertJoinEventFor(e);
	}
}

void SpiralTreeObstructedAlgorithm::VertexEvent::insertJoinEventFor(
    SweepCircle::EdgeMap::iterator rightEdge) {
	SweepInterval* interval = rightEdge->second->nextInterval();
	if (interval->previousBoundary() == nullptr || interval->nextBoundary() == nullptr) {
		return;
	}
	if (interval->type() == SweepInterval::Type::OBSTACLE) {
		return;
	}
	std::optional<PolarPoint> vanishingPoint = interval->vanishingPoint(m_alg->m_circle.r());
	if (vanishingPoint) {
		m_alg->m_queue.push(std::make_shared<JoinEvent>(*vanishingPoint, rightEdge->second,
		                                                std::next(rightEdge)->second, m_alg));
		std::cout << "(inserted join event at r = " << vanishingPoint->r() << ")" << std::endl;
	} else {
		std::cout << "(ignored join event for ["
		          << interval->previousBoundary()->shape().nearEndpoint().toCartesian();
		if (interval->previousBoundary()->shape().farEndpoint()) {
			std::cout << " – " << interval->previousBoundary()->shape().farEndpoint()->toCartesian();
		}
		std::cout << "]  →  [" << interval->nextBoundary()->shape().nearEndpoint().toCartesian();
		if (interval->nextBoundary()->shape().farEndpoint()) {
			std::cout << " – " << interval->nextBoundary()->shape().farEndpoint()->toCartesian();
		}
		std::cout << "])" << std::endl;
	}
}

SpiralTreeObstructedAlgorithm::JoinEvent::JoinEvent(PolarPoint position,
                                                    std::shared_ptr<SweepEdge> rightEdge,
                                                    std::shared_ptr<SweepEdge> leftEdge,
                                                    SpiralTreeObstructedAlgorithm* alg)
    : Event(position, Type::JOIN, alg), m_rightEdge(rightEdge), m_leftEdge(leftEdge) {}

void SpiralTreeObstructedAlgorithm::JoinEvent::handle() {
	using enum SweepInterval::Type;
	using enum SweepEdgeShape::Type;

	std::cout << "> handling join event" << std::endl;
	m_alg->m_debugPainting->setMode(renderer::GeometryRenderer::stroke);
	m_alg->m_debugPainting->setStroke(Color{0, 120, 240}, 1);
	m_alg->m_debugPainting->draw(m_alg->m_tree->rootPosition() +
	                             (m_position.toCartesian() - CGAL::ORIGIN));
	m_alg->m_debugPainting->drawText(
	    m_alg->m_tree->rootPosition() + (m_position.toCartesian() - CGAL::ORIGIN), "join");

	SweepInterval* previousInterval = m_rightEdge->previousInterval();
	SweepInterval* interval = m_rightEdge->nextInterval();
	SweepInterval* nextInterval = m_leftEdge->nextInterval();

	if (previousInterval->type() == OBSTACLE && nextInterval->type() == OBSTACLE) {
		// ignore, this is handled by a vertex event

	} else if (previousInterval->type() == REACHABLE && nextInterval->type() == REACHABLE) {
		// simply merge the intervals into a big reachable interval
		auto result = m_alg->m_circle.mergeToInterval(*m_rightEdge, *m_leftEdge);
		result.mergedInterval->setType(REACHABLE);

	} else if (previousInterval->type() == OBSTACLE) {
		// right side is obstacle
		auto result = m_alg->m_circle.mergeToEdge(*m_rightEdge, *m_leftEdge, m_rightEdge);

	} else if (nextInterval->type() == OBSTACLE) {
		// left side is obstacle
		auto result = m_alg->m_circle.mergeToEdge(*m_rightEdge, *m_leftEdge, m_leftEdge);
	}
}

SpiralTreeObstructedAlgorithm::SpiralTreeObstructedAlgorithm(std::shared_ptr<SpiralTree> tree)
    : m_tree(tree), m_debugPainting(std::make_shared<renderer::PaintingRenderer>()) {}

void SpiralTreeObstructedAlgorithm::run() {

	// insert all terminals into the event queue
	for (const std::shared_ptr<Node>& node : m_tree->nodes()) {
		if (node->m_position.r() > 0) {
			m_queue.push(std::make_shared<NodeEvent>(node->m_position, this));
		}
	}
	for (SpiralTree::Obstacle& obstacle : m_tree->obstacles()) {
		for (auto e = obstacle.begin(); e != obstacle.end(); e++) {
			std::shared_ptr<SweepEdge> e1 = *e;
			std::shared_ptr<SweepEdge> e2 = ++e == obstacle.end() ? *obstacle.begin() : *e;
			m_queue.push(std::make_shared<VertexEvent>(e2->shape().start(), e1, e2, this));
			e--;
		}
	}

	m_circle.print();
	int eventCount = 0; // TODO debug: limit number of events handled
	// main loop, handle all events
	while (!m_queue.empty() && eventCount++ < 100) {
		std::shared_ptr<Event> event = m_queue.top();
		m_queue.pop();
		m_circle.firstInterval().paintSweepShape(*m_debugPainting, m_circle.r(), event->r());
		for (const auto& edge : m_circle.edges()) {
			edge.second->nextInterval()->paintSweepShape(*m_debugPainting, m_circle.r(), event->r());
		}
		m_circle.grow(event->r());
		m_circle.print();
		event->handle();
		m_circle.print();
		assert(m_circle.isValid());
	}
}

std::shared_ptr<renderer::GeometryPainting> SpiralTreeObstructedAlgorithm::debugPainting() {
	return m_debugPainting;
}

} // namespace cartocrow::flow_map