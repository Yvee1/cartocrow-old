#include "../catch.hpp"

#include <filesystem>

#include "cartocrow/core/region_map.h"

using namespace cartocrow;

TEST_CASE("Reading regions from an Ipe file") {
	RegionMap regions = ipeToRegionMap(std::filesystem::path("data/test_region_map.ipe"));
	REQUIRE(regions.size() == 2);
	REQUIRE(regions.contains("R1"));
	REQUIRE(regions.contains("R2"));

	Region r1 = regions["R1"];
	Region r2 = regions["R2"];

	CHECK(r1.color.r == 255);
	CHECK(r1.color.g == 255);
	CHECK(r1.color.b == 0);

	CHECK(r1.shape.number_of_polygons_with_holes() == 1);

	CHECK(r2.color.r == 0);
	CHECK(r2.color.g == 255);
	CHECK(r2.color.b == 255);

	CHECK(r2.shape.number_of_polygons_with_holes() == 2);
}

TEST_CASE("Reading a region with a hole from an Ipe file") {
	RegionMap regions = ipeToRegionMap(std::filesystem::path("data/test_region_map_hole.ipe"));
	REQUIRE(regions.size() == 1);
	REQUIRE(regions.contains("R1"));
	Region r1 = regions["R1"];
	REQUIRE(r1.shape.number_of_polygons_with_holes() == 2);
	std::vector<PolygonWithHoles<Exact>> polygons;
	r1.shape.polygons_with_holes(std::back_inserter(polygons));
	REQUIRE(polygons.size() == 2);
	CHECK(((polygons[0].number_of_holes() == 1 && polygons[1].number_of_holes() == 0) ||
	       (polygons[0].number_of_holes() == 0 && polygons[1].number_of_holes() == 1)));
}

TEST_CASE("Reading a region with a symbolic color from an Ipe file") {
	RegionMap regions =
	    ipeToRegionMap(std::filesystem::path("data/test_region_map_symbolic_color.ipe"));
	REQUIRE(regions.size() == 1);
	REQUIRE(regions.contains("R1"));
	Region r1 = regions["R1"];
	CHECK(r1.color.r == 255);
	CHECK(r1.color.g == 0);
	CHECK(r1.color.b == 0);
}

TEST_CASE("Reading an Ipe file with a non-simple polygon (should throw)") {
	CHECK_THROWS_WITH(ipeToRegionMap(std::filesystem::path("data/test_region_map_non_simple.ipe")),
	                  "Encountered non-simple polygon");
}

TEST_CASE("Reading an Ipe file with a region containing no label (should throw)") {
	CHECK_THROWS_WITH(
	    ipeToRegionMap(std::filesystem::path("data/test_region_map_missing_label.ipe")),
	    "Encountered region without a label");
}

TEST_CASE("Reading an Ipe file with a region containing two labels (should throw)") {
	CHECK_THROWS_WITH(ipeToRegionMap(std::filesystem::path("data/test_region_map_two_labels.ipe")),
	                  "Encountered region with more than one label");
}
