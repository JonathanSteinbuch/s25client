// Copyright (c) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
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

#pragma once

#include "FlagType.h"
#include <cstdint>

/// Visibility of a node
enum class Visibility : uint8_t
{
    Invisible,
    FogOfWar,
    Visible
};
constexpr auto maxEnumValue(Visibility)
{
    return Visibility::Visible;
}

/// Direction from a point where a road can go. Opposites are stored in neighbors
enum class RoadDir
{
    East,
    SouthEast,
    SouthWest
};
constexpr auto maxEnumValue(RoadDir)
{
    return RoadDir::SouthWest;
}

/// Type of the road "owned" by a point
enum class PointRoad : unsigned char
{
    None,
    Normal,
    Donkey,
    Boat
};
constexpr auto maxEnumValue(PointRoad)
{
    return PointRoad::Boat;
}
