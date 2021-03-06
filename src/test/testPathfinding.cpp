// Copyright (c) 2016 - 2017 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Return To The Roots.
//
// Return To The Roots is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Return To The Roots is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Return To The Roots. If not, see <http://www.gnu.org/licenses/>.

#include "rttrDefines.h" // IWYU pragma: keep
#include "CreateEmptyWorld.h"
#include "PointOutput.h"
#include "WorldFixture.h"
#include "nodeObjs/noGranite.h"
#include "gameTypes/Direction_Output.h"
#include "gameData/GameConsts.h"
#include "test/initTestHelpers.h"
#include <boost/assign/std/vector.hpp>
#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>
#include <vector>

using namespace boost::assign;

// Tests are designed to check for every possible direction and terrain distribution
// To understand what is tested check test/testPathfindingIllustration.png. It consists
// of 4 test cases which are partially divided in variations a-c and possibly marked with
// differently colored start/end points if multiple tests are performed on the same landscape
BOOST_AUTO_TEST_SUITE(PathfindingSuite)

typedef WorldFixture<CreateEmptyWorld, 0> WorldFixtureEmpty0P;
typedef WorldFixture<CreateEmptyWorld, 1> WorldFixtureEmpty1P;

/// Sets all terrain to the given terrain
void clearWorld(GameWorldGame& world, TerrainType terrain)
{
    RTTR_FOREACH_PT(MapPoint, world.GetSize())
    {
        MapNode& node = world.GetNodeWriteable(pt);
        node.t1 = node.t2 = terrain;
    }
}

void setupTestcase1(GameWorldGame& world, const MapPoint& startPt, TerrainType tBlue, TerrainType tWhite)
{
    // test case 1: Everything is covered in blue terrain (e.g. water) which is walkable on the shore
    // so the white terrain creates all possible shore orientations
    clearWorld(world, tBlue);
    // Create the white terrain from left to right
    // curPt stores the current point on the path
    world.GetNodeWriteable(world.GetNeighbour(startPt, Direction::NORTHWEST)).t2 = tWhite;
    MapPoint curPt = world.GetNeighbour(startPt, Direction::NORTHEAST);
    world.GetNodeWriteable(curPt).t2 = tWhite; //-V807
    curPt = world.GetNeighbour(curPt, Direction::SOUTHEAST);
    world.GetNodeWriteable(curPt).t1 = tWhite;
    curPt = world.GetNeighbour(curPt, Direction::SOUTHEAST);
    world.GetNodeWriteable(world.GetNeighbour(curPt, Direction::NORTHEAST)).t1 = tWhite;
    curPt = world.GetNeighbour(curPt, Direction::NORTHEAST);
    world.GetNodeWriteable(world.GetNeighbour(curPt, Direction::NORTHEAST)).t1 = tWhite;
    curPt = world.GetNeighbour(curPt, Direction::EAST);
    world.GetNodeWriteable(curPt).t2 = tWhite;
}

void setupTestcase2to4(GameWorldGame& world, const MapPoint& startPt, TerrainType tWalkable, TerrainType tOther, bool bothTerrain,
                       Direction dir)
{
    // test cases 2-4: Everything covered in walkable terrain (white) and we want to walk 3 steps into a specified direction
    // after 1 step we encounter other terrain at both(2) or the left(3/4) side of the path
    // Note that due to the design of the test cases we have the terrain always on the left side
    // as we go right for test case 3 and left for test case 4
    clearWorld(world, tWalkable);
    // pt where we encounter the other terrain
    MapPoint terrainPt = world.GetNeighbour(startPt, dir);
    setLeftTerrain(world, terrainPt, dir, tOther);
    if(bothTerrain)
        setRightTerrain(world, terrainPt, dir, tOther);
}

BOOST_FIXTURE_TEST_CASE(WalkStraight, WorldFixtureEmpty0P)
{
    std::vector<Direction> testDirections;
    testDirections += Direction::EAST, Direction::SOUTHEAST, Direction::NORTHEAST;
    testDirections += Direction::WEST, Direction::SOUTHWEST, Direction::NORTHWEST;
    std::vector<TerrainType> friendlyTerrains;
    friendlyTerrains += TT_DESERT, TT_MEADOW_FLOWERS, TT_MOUNTAIN1, TT_MOUNTAIN2, TT_MOUNTAIN3, TT_MOUNTAIN4, TT_SAVANNAH, TT_MEADOW1,
      TT_MEADOW2, TT_MEADOW3, TT_STEPPE, TT_MOUNTAINMEADOW, TT_BUILDABLE_WATER, TT_BUILDABLE_MOUNTAIN;

    const MapPoint startPt(0, 6);

    BOOST_FOREACH(TerrainType friendlyTerrain, friendlyTerrains)
    {
        clearWorld(world, friendlyTerrain);
        BOOST_FOREACH(Direction dir, testDirections)
        {
            // 3 steps in dir
            MapPoint endPt(startPt);
            for(unsigned i = 0; i < 3; i++)
                endPt = world.GetNeighbour(endPt, dir);
            unsigned length;
            // Must be able to go there directly
            BOOST_REQUIRE_NE(world.FindHumanPath(startPt, endPt, 99, false, &length), INVALID_DIR);
            BOOST_REQUIRE_EQUAL(length, 3u);
            // Inverse route
            BOOST_REQUIRE_NE(world.FindHumanPath(endPt, startPt, 99, false, &length), INVALID_DIR);
            BOOST_REQUIRE_EQUAL(length, 3u);
        }
    }
}

BOOST_FIXTURE_TEST_CASE(WalkAlongCoast, WorldFixtureEmpty0P)
{
    const MapPoint startPt(5, 2);
    setupTestcase1(world, startPt, TT_WATER, TT_STEPPE);
    // 4 steps right
    MapPoint endPt = world.MakeMapPoint(Position(startPt.x + 4, startPt.y));
    unsigned length;
    std::vector<Direction> route;
    // Forward route
    BOOST_REQUIRE_NE(world.FindHumanPath(startPt, endPt, 99, false, &length, &route), INVALID_DIR);
    BOOST_REQUIRE_EQUAL(length, 6u);
    BOOST_REQUIRE_EQUAL(route.size(), 6u);
    std::vector<Direction> expectedRoute;
    expectedRoute += Direction::NORTHEAST, Direction::SOUTHEAST, Direction::SOUTHEAST, Direction::NORTHEAST, Direction::EAST,
      Direction::EAST;
    RTTR_REQUIRE_EQUAL_COLLECTIONS(route, expectedRoute);
    // Inverse route
    BOOST_REQUIRE_NE(world.FindHumanPath(endPt, startPt, 99, false, &length, &route), INVALID_DIR);
    BOOST_REQUIRE_EQUAL(length, 6u);
    BOOST_REQUIRE_EQUAL(route.size(), 6u);
    std::vector<Direction> expectedRevRoute;
    BOOST_REVERSE_FOREACH(Direction dir, expectedRoute)
    {
        expectedRevRoute.push_back(dir + 3u);
    }
    RTTR_REQUIRE_EQUAL_COLLECTIONS(route, expectedRevRoute);
}

BOOST_FIXTURE_TEST_CASE(CrossTerrain, WorldFixtureEmpty1P)
{
    // For road checking all the points must belong to us (or any owner)
    RTTR_FOREACH_PT(MapPoint, world.GetSize())
    {
        world.SetOwner(pt, 1);
    }
    // Start far enough away from the HQ in the middle
    const MapPoint startPt(1, 2);
    std::vector<Direction> testDirections;
    // Test cases 2         a)                 b)                     c)
    testDirections += Direction::EAST, Direction::SOUTHEAST, Direction::NORTHEAST;
    std::vector<TerrainType> deepWaterTerrains;
    deepWaterTerrains += TT_WATER, TT_WATER_NOSHIP, TT_SWAMPLAND;
    BOOST_FOREACH(TerrainType deepWater, deepWaterTerrains)
    {
        BOOST_FOREACH(Direction dir, testDirections)
        {
            setupTestcase2to4(world, startPt, TT_STEPPE, deepWater, true, dir);
            // 3 steps in dir
            MapPoint endPt(startPt);
            for(unsigned i = 0; i < 3; i++)
                endPt = world.GetNeighbour(endPt, dir);
            // We can't go directly so 1 step detour
            unsigned length;
            BOOST_REQUIRE_NE(world.FindHumanPath(startPt, endPt, 99, false, &length), INVALID_DIR);
            BOOST_REQUIRE_EQUAL(length, 4u);
            // Inverse route
            BOOST_REQUIRE_NE(world.FindHumanPath(endPt, startPt, 99, false, &length), INVALID_DIR);
            BOOST_REQUIRE_EQUAL(length, 4u);
            // But road must be constructible
            world.SetFlag(startPt, 0);
            std::vector<Direction> roadRoute(3, dir);
            world.BuildRoad(0, false, startPt, roadRoute);
            Direction revDir(dir.toUInt() + 3);
            BOOST_REQUIRE_EQUAL(world.GetPointRoad(startPt, dir), 1u);
            BOOST_REQUIRE_EQUAL(world.GetPointRoad(endPt, revDir), 1u);
            world.DestroyFlag(endPt, 0);
            // Reverse direction
            std::vector<Direction> roadRouteRev(3, revDir);
            world.SetFlag(endPt, 0);
            world.BuildRoad(0, false, endPt, roadRouteRev);
            BOOST_REQUIRE_EQUAL(world.GetPointRoad(startPt, dir), 1u);
            BOOST_REQUIRE_EQUAL(world.GetPointRoad(endPt, revDir), 1u);
            world.DestroyFlag(startPt, 0);
            world.DestroyFlag(endPt, 0);
        }
    }
}

BOOST_FIXTURE_TEST_CASE(DontPassTerrain, WorldFixtureEmpty1P)
{
    // For road checking all the points must belong to us (or any owner)
    RTTR_FOREACH_PT(MapPoint, world.GetSize())
    {
        world.SetOwner(pt, 1);
    }
    // Start far enough away from the HQ in the middle
    const MapPoint startPt(1, 2);
    std::vector<Direction> testDirections;
    // Test cases 3         a)                 b)                     c)
    testDirections += Direction::EAST, Direction::SOUTHEAST, Direction::NORTHEAST;
    // Test cases 4         a)                 b)                     c)
    testDirections += Direction::WEST, Direction::SOUTHWEST, Direction::NORTHWEST;
    std::vector<TerrainType> deadlyTerrains;
    deadlyTerrains += TT_SNOW, TT_LAVA, TT_LAVA2, TT_LAVA3, TT_LAVA4;

    BOOST_FOREACH(TerrainType deadlyTerrain, deadlyTerrains)
    {
        BOOST_FOREACH(Direction dir, testDirections)
        {
            setupTestcase2to4(world, startPt, TT_STEPPE, deadlyTerrain, false, dir);
            // 3 steps in dir
            MapPoint endPt(startPt);
            for(unsigned i = 0; i < 3; i++)
                endPt = world.GetNeighbour(endPt, dir);
            MapPoint curStartPt(startPt);
            // We test the green points first and the yellow points second
            for(int i = 0; i < 2; i++)
            {
                unsigned length;
                BOOST_REQUIRE_NE(world.FindHumanPath(curStartPt, endPt, 99, false, &length), INVALID_DIR);
                BOOST_REQUIRE_EQUAL(length, 4u);
                // Inverse route
                BOOST_REQUIRE_NE(world.FindHumanPath(endPt, curStartPt, 99, false, &length), INVALID_DIR);
                BOOST_REQUIRE_EQUAL(length, 4u);
                // No road must be constructible
                world.SetFlag(startPt, 0);
                std::vector<Direction> roadRoute(3, dir);
                world.BuildRoad(0, false, startPt, roadRoute);
                Direction revDir(dir + 3u);
                BOOST_REQUIRE_EQUAL(world.GetPointRoad(startPt, dir), 0u);
                BOOST_REQUIRE_EQUAL(world.GetPointRoad(endPt, revDir), 0u);
                world.DestroyFlag(startPt, 0);
                // Reverse direction
                std::vector<Direction> roadRouteRev(3, revDir);
                world.SetFlag(endPt, 0);
                world.BuildRoad(0, false, endPt, roadRouteRev);
                BOOST_REQUIRE_EQUAL(world.GetPointRoad(startPt, revDir), 0u);
                BOOST_REQUIRE_EQUAL(world.GetPointRoad(endPt, dir), 0u);
                world.DestroyFlag(endPt, 0);
                // Switch to yellow points. They are placed to be one step left than the direction
                curStartPt = world.GetNeighbour(curStartPt, dir - 1u);
                endPt = world.GetNeighbour(endPt, dir - 1u);
            }
        }
    }
}

BOOST_FIXTURE_TEST_CASE(BlockedPaths, WorldFixtureEmpty0P)
{
    MapPoint startPt(3, 6);
    // Create a circle of stones so the path is completely blocked
    std::vector<MapPoint> surroundingPts = world.GetPointsInRadius(startPt, 1);
    BOOST_FOREACH(const MapPoint& pt, surroundingPts)
        world.SetNO(pt, new noGranite(GT_1, 1));
    std::vector<MapPoint> surroundingPts2;
    for(unsigned i = 0; i < 12; i++)
        surroundingPts2.push_back(world.GetNeighbour2(startPt, i));
    BOOST_FOREACH(const MapPoint& pt, surroundingPts2)
        BOOST_REQUIRE_EQUAL(world.FindHumanPath(startPt, pt), INVALID_DIR);
    // Allow left exit
    world.DestroyNO(surroundingPts[0]);
    BOOST_REQUIRE_EQUAL(world.FindHumanPath(startPt, surroundingPts2[0]), 0);
}

BOOST_AUTO_TEST_SUITE_END()
