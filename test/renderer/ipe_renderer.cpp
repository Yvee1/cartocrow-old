#include "../catch.hpp"

#include "../../cartocrow/core/ipe_reader.h"
#include "../../cartocrow/renderer/geometry_painting.h"
#include "../../cartocrow/renderer/ipe_renderer.h"

#include <ipedoc.h>

#include <filesystem>
#include <ipeshape.h>

using namespace cartocrow;

TEST_CASE("Exporting marks to Ipe") {
	class TestPainting : public renderer::GeometryPainting {
		void paint(renderer::GeometryRenderer& renderer) {
			renderer.draw(Point<Exact>(0, 0));
			renderer.draw(Point<Exact>(2, 1));
		}
	};
	TestPainting painting;
	renderer::IpeRenderer renderer(painting);
	std::filesystem::path path = std::filesystem::temp_directory_path() / "test.ipe";
	renderer.save(path);

	std::shared_ptr<ipe::Document> result = IpeReader::loadIpeFile(path);
	REQUIRE(result->countPages() == 1);
	REQUIRE(result->page(0)->count() == 2);
	REQUIRE(result->page(0)->object(0)->type() == ipe::Object::Type::EReference);
	CHECK(result->page(0)->object(0)->asReference()->position() == ipe::Vector(0, 0));
	REQUIRE(result->page(0)->object(1)->type() == ipe::Object::Type::EReference);
	CHECK(result->page(0)->object(1)->asReference()->position() == ipe::Vector(2, 1));
}

TEST_CASE("Exporting a line segment to Ipe") {
	class TestPainting : public renderer::GeometryPainting {
		void paint(renderer::GeometryRenderer& renderer) {
			renderer.draw(Segment<Exact>(Point<Exact>(2, 3), Point<Exact>(1, 4)));
		}
	};
	TestPainting painting;
	renderer::IpeRenderer renderer(painting);
	std::filesystem::path path = std::filesystem::temp_directory_path() / "test.ipe";
	renderer.save(path);

	std::shared_ptr<ipe::Document> result = IpeReader::loadIpeFile(path);
	REQUIRE(result->countPages() == 1);
	REQUIRE(result->page(0)->count() == 1);
	REQUIRE(result->page(0)->object(0)->type() == ipe::Object::Type::EPath);
	const ipe::Path* ipePath = result->page(0)->object(0)->asPath();
	REQUIRE(ipePath->shape().countSubPaths() == 1);
	REQUIRE(ipePath->shape().subPath(0)->type() == ipe::SubPath::Type::ECurve);
	const ipe::Curve* ipeCurve = ipePath->shape().subPath(0)->asCurve();
	REQUIRE(ipeCurve->countSegments() == 1);
	REQUIRE(ipeCurve->segment(0).type() == ipe::CurveSegment::Type::ESegment);
	CHECK(ipeCurve->segment(0).cp(0) == ipe::Vector(2, 3));
	CHECK(ipeCurve->segment(0).cp(1) == ipe::Vector(1, 4));
}