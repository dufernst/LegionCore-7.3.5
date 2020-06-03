/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Position.h"
#include "ByteBuffer.h"
#include "GridDefines.h"
#include <G3D/LineSegment.h>

Position::Position(float x, float y, float z, float o, float h) : m_positionX(x), m_positionY(y), m_positionZ(z), m_positionH(h), m_orientation(NormalizeOrientation(o))
{
}

Position::Position(Position const& loc)
{
    Position::Relocate(loc);
}

Position::Position(DBCPosition4D const& dbcLoc)
{
    Position loc{ dbcLoc.X, dbcLoc.Y, dbcLoc.Z, dbcLoc.O };
    Position::Relocate(loc);
}

void Position::operator+=(Position pos)
{
    m_positionX += pos.m_positionX;
    m_positionY += pos.m_positionY;
    m_positionZ += pos.m_positionZ;
    m_positionH += pos.m_positionH;
    m_orientation += pos.m_orientation;
}

void Position::Clear()
{
    m_positionX = 0.0f;
    m_positionY = 0.0f;
    m_positionZ = 0.0f;
    m_positionH = 0.0f;
    SetOrientation(0.0f);
}

void Position::Relocate(float x, float y)
{
    m_positionX = x;
    m_positionY = y;
}

void Position::Relocate(float x, float y, float z)
{
    m_positionX = x;
    m_positionY = y;
    m_positionZ = z;
}

void Position::Relocate(float x, float y, float z, float orientation)
{
    m_positionX = x;
    m_positionY = y;
    m_positionZ = z;
    SetOrientation(orientation);
}

void Position::Relocate(float x, float y, float z, float orientation, float positionH)
{
    m_positionX = x;
    m_positionY = y;
    m_positionZ = z;
    SetOrientation(orientation);
    m_positionH = positionH;
}

void Position::Relocate(Position const& pos)
{
    m_positionX = pos.m_positionX;
    m_positionY = pos.m_positionY;
    m_positionZ = pos.m_positionZ;
    m_positionH = pos.m_positionH;
    SetOrientation(pos.m_orientation);
}

void Position::Relocate(Position const* pos)
{
    m_positionX = pos->m_positionX;
    m_positionY = pos->m_positionY;
    m_positionZ = pos->m_positionZ;
    m_positionH = pos->m_positionH;
    SetOrientation(pos->m_orientation);
}

void Position::SetPosition(DBCPosition2D pos)
{
    m_positionX = pos.X;
    m_positionY = pos.Y;
}

void Position::SetPosition(DBCPosition3D pos)
{
    m_positionX = pos.X;
    m_positionY = pos.Y;
    m_positionZ = pos.Z;
}

void Position::SetPosition(DBCPosition4D pos)
{
    m_positionX = pos.X;
    m_positionY = pos.Y;
    m_positionZ = pos.Z;
    SetOrientation(pos.O);
}

bool Position::IsWithinBox(Position const& center, float xradius, float yradius, float zradius) const
{
    // rotate the WorldObject position instead of rotating the whole cube, that way we can make a simplified
    // is-in-cube check and we have to calculate only one point instead of 4

    // 2PI = 360*, keep in mind that ingame orientation is counter-clockwise
    double rotation = 2 * M_PI - center.GetOrientation();
    double sinVal = std::sin(rotation);
    double cosVal = std::cos(rotation);

    float BoxDistX = GetPositionX() - center.GetPositionX();
    float BoxDistY = GetPositionY() - center.GetPositionY();

    float rotX = float(center.GetPositionX() + BoxDistX * cosVal - BoxDistY * sinVal);
    float rotY = float(center.GetPositionY() + BoxDistY * cosVal + BoxDistX * sinVal);

    // box edges are parallel to coordiante axis, so we can treat every dimension independently :D
    if ((std::fabs(rotX - center.GetPositionX()) > xradius) || (std::fabs(rotY - center.GetPositionY()) > yradius) || (std::fabs(GetPositionZ() - center.GetPositionZ()) > zradius))
        return false;

    return true;
}

bool Position::HasInLine(WorldObject const* target, float width) const
{
    if (!HasInArc(M_PI, target))
        return false;
    width += target->GetObjectSize();
    float angle = GetRelativeAngle(target);
    return fabs(sin(angle)) * GetExactDist2d(target->GetPositionX(), target->GetPositionY()) < width;
}

bool Position::IsInDegreesRange(float x, float y, float degresA, float degresB, bool relative/* = false*/) const
{
    float angel = GetDegreesAngel(x, y, relative);
    return angel >= degresA && angel <= degresB;
}

float Position::GetDegreesAngel(float x, float y, bool relative) const
{
    float angel = relative ? GetRelativeAngle(x, y) : GetAngle(x, y);
    return NormalizeOrientation(angel) * M_RAD;
}

Position Position::GetRandPointBetween(Position const& B) const
{
    float Lambda = urand(0.0f, 100.0f) / 100.0f;
    float X = (B.GetPositionX() + Lambda * GetPositionX()) / (1 + Lambda);
    float Y = (B.GetPositionY() + Lambda * GetPositionY()) / (1 + Lambda);
    float Z = (B.GetPositionZ() + Lambda * GetPositionZ()) / (1 + Lambda); //Z should be updated by Vmap

    Position result;
    result.Relocate(X, Y, Z, 0.0f);
    return result;
}

void Position::SimplePosXYRelocationByAngle(Position &pos, float dist, float angle, bool relative) const
{
    if (!relative)
        angle += GetOrientation();

    pos.m_positionX = m_positionX + dist * std::cos(angle);
    pos.m_positionY = m_positionY + dist * std::sin(angle);
    pos.m_positionZ = m_positionZ;

    // Prevent invalid coordinates here, position is unchanged
    if (!Trinity::IsValidMapCoord(pos.m_positionX, pos.m_positionY))
    {
        pos.Relocate(this);
        TC_LOG_FATAL(LOG_FILTER_GENERAL, "Position::SimplePosXYRelocationByAngle invalid coordinates X: %f and Y: %f were passed!", pos.m_positionX, pos.m_positionY);
        return;
    }

    Trinity::NormalizeMapCoord(pos.m_positionX);
    Trinity::NormalizeMapCoord(pos.m_positionY);
    pos.SetOrientation(GetOrientation());
}

bool Position::IsInDist2d(float x, float y, float dist) const
{
    return GetExactDist2dSq(x, y) < dist * dist;
}

bool Position::IsInDist2d(Position const* pos, float dist) const
{
    return GetExactDist2dSq(pos) < dist * dist;
}

bool Position::IsInDist(float x, float y, float z, float dist) const
{
    return GetExactDistSq(x, y, z) < dist * dist;
}

bool Position::IsInDist(Position const* pos, float dist) const
{
    return GetExactDistSq(pos) < dist * dist;
}

bool Position::IsLinesCross(Position const &pos11, Position const &pos12, Position const &pos21, Position const &pos22, Position * dest) const
{
    //Line 1
    G3D::Vector2 p11(pos11.GetPositionX(), pos11.GetPositionY());
    G3D::Vector2 p12(pos12.GetPositionX(), pos12.GetPositionY());
    G3D::LineSegment2D line1 = G3D::LineSegment2D::fromTwoPoints(p11, p12);
    //Line 2
    G3D::Vector2 p21(pos21.GetPositionX(), pos21.GetPositionY());
    G3D::Vector2 p22(pos22.GetPositionX(), pos22.GetPositionY());
    G3D::LineSegment2D line2 = G3D::LineSegment2D::fromTwoPoints(p21, p22);

    G3D::Vector2 result = line1.intersection(line2);
    //check line
    if (result == G3D::Vector2::inf())
        return false;

    if (dest)
    {
        dest->m_positionX = result.x;
        dest->m_positionY = result.y;
    }

    return true;
}

bool Position::IsPointInBox(Position const& centerBox, std::vector<Position> box, Position const& point) const
{
    for (uint32 j = 0; j < box.size(); ++j)
    {
        uint32 first = j;
        uint32 second = j + 1;
        if (second >= box.size())
            second = 0;

        if (centerBox.IsLinesCross(centerBox, point, box[first], box[second]))
            return false;
    }
    return true;
}

void Position::GeneratePointInBox(Position centerBox, std::vector<Position> box, std::vector<Position>& points, uint32 count) const
{
    if (box.size() <= 2) // Min point 3
        return;

    float minX = box[0].m_positionX, minY = box[0].m_positionY, maxX = box[0].m_positionX, maxY = box[0].m_positionY;
    for (uint32 i = 0; i < box.size(); ++i)
    {
        if (minX > box[0].m_positionX)
            minX = box[0].m_positionX;
        if (maxX < box[0].m_positionX)
            maxX = box[0].m_positionX;

        if (minY > box[0].m_positionY)
            minY = box[0].m_positionY;
        if (maxY < box[0].m_positionY)
            maxY = box[0].m_positionY;
    }

    for (uint32 i = 0; i < count;)
    {
        auto pos = Position{ frand(minX, maxX), frand(minY, maxY), centerBox.m_positionZ };

        auto canAdd = true;
        for (uint32 j = 0; j < box.size(); ++j)
        {
            uint32 first = j;
            uint32 second = j + 1;
            if (second >= box.size())
                second = 0;

            if (centerBox.IsLinesCross(centerBox, pos, box[first], box[second]))
                canAdd = false;
        }

        if (!canAdd)
            continue;

        points.emplace_back(pos);
        ++i;
    }
}

void Position::GenerateNonDuplicatePoints(std::list<Position>& randPosList, Position const& centerPos, uint8 maxPoint, float randMin, float randMax, float minDist) const
{
    Position pos;
    uint8 traiCount = 0;

    while (randPosList.size() < maxPoint)
    {
        auto badPos = false;
        centerPos.SimplePosXYRelocationByAngle(pos, frand(randMin, randMax), frand(0.0f, 6.28f));
        ++traiCount;

        for (const auto& _pos : randPosList)
        {
            if (pos.GetExactDist(&_pos) <= minDist)
            {
                badPos = true;
                break;
            }
        }

        if (!badPos || traiCount > (maxPoint * 10))
            randPosList.push_back(pos);
    }
}

float Position::NormalizeOrientation(float o)
{
    // fmod only supports positive numbers. Thus we have
    // to emulate negative numbers
    if (o < 0)
    {
        float mod = o * -1;
        mod = fmod(mod, 2.0f * static_cast<float>(M_PI));
        mod = -mod + 2.0f * static_cast<float>(M_PI);
        return mod;
    }
    return fmod(o, 2.0f * static_cast<float>(M_PI));
}

float Position::NormalizePitch(float o)
{
    if (o > -M_PI && o < M_PI)
        return o;

    o = NormalizeOrientation(o + M_PI) - M_PI;
    return o;
}

void Position::RelocateOffset(Position const& offset)
{
    m_positionX = GetPositionX() + (offset.GetPositionX() * std::cos(GetOrientation()) + offset.GetPositionY() * std::sin(GetOrientation() + M_PI));
    m_positionY = GetPositionY() + (offset.GetPositionY() * std::cos(GetOrientation()) + offset.GetPositionX() * std::sin(GetOrientation()));
    m_positionZ = GetPositionZ() + offset.GetPositionZ();
    SetOrientation(GetOrientation() + offset.GetOrientation());
}

void Position::SetOrientation(float orientation)
{
    m_orientation = NormalizeOrientation(orientation);
}

void Position::SetPositionH(float positionH)
{
    m_positionH = positionH;
}

float Position::GetPositionX() const
{
    return m_positionX;
}

float Position::GetPositionY() const
{
    return m_positionY;
}

float Position::GetPositionZ() const
{
    return m_positionZ;
}

float Position::GetPositionH() const
{
    return m_positionH;
}

float Position::GetOrientation() const
{
    return m_orientation;
}

float Position::GetPositionZH() const
{
    return m_positionZ - m_positionH;
}

Position Position::GetPosition() const
{
    return *this;
}

void Position::GetPosition(float& x, float& y, Transport* /*transport*/) const
{
    x = m_positionX;
    y = m_positionY;
}

void Position::GetPosition(float& x, float& y, float& z, Transport* /*transport*/) const
{
    x = m_positionX;
    y = m_positionY;
    z = GetPositionZH();
}

void Position::GetPosition(float& x, float& y, float& z, float& o, Transport* /*transport*/) const
{
    x = m_positionX;
    y = m_positionY;
    z = GetPositionZH();
    o = m_orientation;
}

void Position::GetPosition(Position* pos, Transport* /*transport*/) const
{
    if (pos)
        pos->Relocate(m_positionX, m_positionY, GetPositionZH(), m_orientation);
}

Position::Streamer<Position::XY> Position::PositionXYStream()
{
    return Streamer<XY>(*this);
}

Position::ConstStreamer<Position::XY> Position::PositionXYStream() const
{
    return ConstStreamer<XY>(*this);
}

Position::Streamer<Position::XYZ> Position::PositionXYZStream()
{
    return Streamer<XYZ>(*this);
}

Position::ConstStreamer<Position::XYZ> Position::PositionXYZStream() const
{
    return ConstStreamer<XYZ>(*this);
}

Position::Streamer<Position::XYZO> Position::PositionXYZOStream()
{
    return Streamer<XYZO>(*this);
}

Position::ConstStreamer<Position::XYZO> Position::PositionXYZOStream() const
{
    return ConstStreamer<XYZO>(*this);
}

Position::Streamer<Position::PackedXYZ> Position::PositionPackedXYZStream()
{
    return Streamer<PackedXYZ>(*this);
}

Position::ConstStreamer<Position::PackedXYZ> Position::PositionPackedXYZStream() const
{
    return ConstStreamer<PackedXYZ>(*this);
}

void Position::GetPositionOffsetTo(Position const& endPos, Position & retOffset) const
{
    float dx = endPos.GetPositionX() - GetPositionX();
    float dy = endPos.GetPositionY() - GetPositionY();

    retOffset.m_positionX = dx * std::cos(GetOrientation()) + dy * std::sin(GetOrientation());
    retOffset.m_positionY = dy * std::cos(GetOrientation()) - dx * std::sin(GetOrientation());
    retOffset.m_positionZ = endPos.GetPositionZ() - GetPositionZ();
    retOffset.SetOrientation(endPos.GetOrientation() - GetOrientation());
}

float Position::GetAngle(Position const* obj) const
{
    if (!obj)
        return 0;

    return GetAngle(obj->GetPositionX(), obj->GetPositionY());
}

// Return angle in range 0..2*pi
float Position::GetAngle(const float x, const float y) const
{
    float dx = x - GetPositionX();
    float dy = y - GetPositionY();

    float ang = atan2(dy, dx);
    ang = (ang >= 0) ? ang : 2 * M_PI + ang;
    return ang;
}

float Position::GetRelativeAngle(Position const* pos) const
{
    return GetAngle(pos) - m_orientation;
}

float Position::GetRelativeAngle(float x, float y) const
{
    return GetAngle(x, y) - m_orientation;
}

void Position::GetSinCos(const float x, const float y, float &vsin, float &vcos) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;

    if (fabs(dx) < 0.001f && fabs(dy) < 0.001f)
    {
        float angle = static_cast<float>(rand_norm())*static_cast<float>(2 * M_PI);
        vcos = std::cos(angle);
        vsin = std::sin(angle);
    }
    else
    {
        float dist = sqrt((dx*dx) + (dy*dy));
        vcos = dx / dist;
        vsin = dy / dist;
    }
}

bool Position::HasInArc(float arc, Position const* obj) const
{
    // always have self in arc
    if (obj == this)
        return true;

    // move arc to range 0.. 2*pi
    arc = NormalizeOrientation(arc);

    float angle = GetAngle(obj);
    angle -= m_orientation;

    // move angle to range -pi ... +pi
    angle = NormalizeOrientation(angle);
    if (angle > M_PI)
        angle -= 2.0f*M_PI;

    float lborder = -1 * (arc / 2.0f);                        // in range -pi..0
    float rborder = (arc / 2.0f);                             // in range 0..pi
    return ((angle >= lborder) && (angle <= rborder));
}

bool Position::IsPositionValid() const
{
    return Trinity::IsValidMapCoord(m_positionX, m_positionY, m_positionZ, m_orientation);
}

float Position::GetExactDist2dSq(float x, float y) const
{
    float dx = m_positionX - x;
    float dy = m_positionY - y;
    return dx * dx + dy * dy;
}

float Position::GetExactDist2d(float x, float y) const
{
    return sqrt(GetExactDist2dSq(x, y));
}

float Position::GetExactDist2dSq(Position const* pos) const
{
    float dx = m_positionX - pos->m_positionX;
    float dy = m_positionY - pos->m_positionY;
    return dx * dx + dy * dy;
}

float Position::GetExactDist2d(Position const* pos) const
{
    return sqrt(GetExactDist2dSq(pos));
}

float Position::GetExactDistSq(float x, float y, float z) const
{
    float dz = GetPositionZH() - z;
    return GetExactDist2dSq(x, y) + dz * dz;
}

float Position::GetExactDist(float x, float y, float z) const
{
    return sqrt(GetExactDistSq(x, y, z));
}

float Position::GetExactDistSq(Position const* pos) const
{
    float dx = m_positionX - pos->m_positionX;
    float dy = m_positionY - pos->m_positionY;
    float dz = GetPositionZH() - pos->GetPositionZH();
    return dx * dx + dy * dy + dz * dz;
}

float Position::GetExactDist(Position const* pos) const
{
    return sqrt(GetExactDistSq(pos));
}

std::string Position::ToString() const
{
    std::stringstream sstr;
    sstr << "X: " << m_positionX << " Y: " << m_positionY << " Z: " << m_positionZ << " O: " << m_orientation;
    return sstr.str();
}

Position Position::operator-(Position const& rkVector) const
{
    return Position(m_positionX - rkVector.m_positionX, m_positionY - rkVector.m_positionY, m_positionZ - rkVector.m_positionZ);
}

float Position::magnitude() const
{
    return ::sqrtf(m_positionX * m_positionX + m_positionY * m_positionY + m_positionZ * m_positionZ);
}

float Position::length() const
{
    return magnitude();
}

WorldLocation::WorldLocation(uint32 _mapid, float _x, float _y, float _z, float _o) : m_mapId(_mapid)
{
    Position::Relocate(_x, _y, _z, _o);
}

WorldLocation::WorldLocation(uint32 _mapid, Position const& pos) : m_mapId(_mapid)
{
    Position::Relocate(pos);
}

WorldLocation::WorldLocation(WorldLocation const& loc) : Position(loc)
{
    WorldRelocate(loc);
}

void WorldLocation::WorldRelocate(WorldLocation const& loc)
{
    m_mapId = loc.GetMapId();
    Relocate(loc);
}

void WorldLocation::WorldRelocate(uint32 _mapId, float _x, float _y, float _z, float _o)
{
    m_mapId = _mapId;
    Relocate(_x, _y, _z, _o);
}

uint32 WorldLocation::GetMapId() const
{
    return m_mapId;
}

void WorldLocation::SetMapId(uint32 _mapid)
{
    m_mapId = _mapid;
}

WorldLocation WorldLocation::GetWorldLocation() const
{
    return *this;
}

ByteBuffer& operator<<(ByteBuffer& buf, Position::ConstStreamer<Position::XY> const& streamer)
{
    buf << streamer.Pos->GetPositionX();
    buf << streamer.Pos->GetPositionY();
    return buf;
}

ByteBuffer& operator >> (ByteBuffer& buf, Position::Streamer<Position::XY> const& streamer)
{
    float x, y;
    buf >> x >> y;
    streamer.Pos->Relocate(x, y);
    return buf;
}

ByteBuffer& operator<<(ByteBuffer& buf, Position::ConstStreamer<Position::XYZ> const& streamer)
{
    buf << streamer.Pos->GetPositionX();
    buf << streamer.Pos->GetPositionY();
    buf << streamer.Pos->GetPositionZ();
    return buf;
}

ByteBuffer& operator >> (ByteBuffer& buf, Position::Streamer<Position::XYZ> const& streamer)
{
    float x, y, z;
    buf >> x >> y >> z;
    streamer.Pos->Relocate(x, y, z);
    return buf;
}

ByteBuffer& operator<<(ByteBuffer& buf, Position::ConstStreamer<Position::XYZO> const& streamer)
{
    buf << streamer.Pos->GetPositionX();
    buf << streamer.Pos->GetPositionY();
    buf << streamer.Pos->GetPositionZ();
    buf << streamer.Pos->GetOrientation();
    return buf;
}

ByteBuffer& operator >> (ByteBuffer& buf, Position::Streamer<Position::XYZO> const& streamer)
{
    float x, y, z, o;
    buf >> x >> y >> z >> o;
    streamer.Pos->Relocate(x, y, z, o);
    return buf;
}

ByteBuffer& operator<<(ByteBuffer& buf, Position::ConstStreamer<Position::PackedXYZ> const& streamer)
{
    buf.appendPackXYZ(streamer.Pos->GetPositionX(), streamer.Pos->GetPositionY(), streamer.Pos->GetPositionZ());
    return buf;
}
