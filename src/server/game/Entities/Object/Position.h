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

#ifndef Trinity_game_Position_h__
#define Trinity_game_Position_h__

#include "Common.h"
#include <vector>

class ByteBuffer;
class Transport;

struct Position
{
    Position(float x = 0, float y = 0, float z = 0, float o = 0, float h = 0);
    Position(Position const& loc);
    Position(DBCPosition4D const& dbcLoc);

    struct XY;
    struct XYZ;
    struct XYZO;
    struct PackedXYZ;

    template<class Tag>
    struct ConstStreamer
    {
        explicit ConstStreamer(Position const& pos) : Pos(&pos) { }
        Position const* Pos;
    };

    template<class Tag>
    struct Streamer
    {
        explicit Streamer(Position& pos) : Pos(&pos) { }
        operator ConstStreamer<Tag>() { return ConstStreamer<Tag>(*Pos); }
        Position* Pos;
    };

    void operator +=(Position pos);

    void Clear();

    virtual void Relocate(float x, float y);
    virtual void Relocate(float x, float y, float z);
    virtual void Relocate(float x, float y, float z, float orientation);
    virtual void Relocate(float x, float y, float z, float orientation, float positionH);

    virtual void Relocate(Position const& pos);
    virtual void Relocate(Position const* pos);

    void SetPosition(DBCPosition2D pos);
    void SetPosition(DBCPosition3D pos);
    void SetPosition(DBCPosition4D pos);

    void RelocateOffset(Position const& offset);
    void SetOrientation(float orientation);
    void SetPositionH(float positionH);

    float GetPositionX() const;
    float GetPositionY() const;
    float GetPositionZ() const;
    float GetPositionH() const;
    float GetOrientation() const;
    float GetPositionZH() const;

    virtual Position GetPosition() const;
    virtual void GetPosition(float& x, float& y, Transport* transport = nullptr) const;
    virtual void GetPosition(float& x, float& y, float& z, Transport* transport = nullptr) const;
    virtual void GetPosition(float& x, float& y, float& z, float& o, Transport* transport = nullptr) const;
    virtual void GetPosition(Position* pos, Transport* transport = nullptr) const;

    Streamer<XY> PositionXYStream();
    ConstStreamer<XY> PositionXYStream() const;
    Streamer<XYZ> PositionXYZStream();
    ConstStreamer<XYZ> PositionXYZStream() const;
    Streamer<XYZO> PositionXYZOStream();
    ConstStreamer<XYZO> PositionXYZOStream() const;
    Streamer<PackedXYZ> PositionPackedXYZStream();
    ConstStreamer<PackedXYZ> PositionPackedXYZStream() const;

    bool IsPositionValid() const;

    float GetExactDist2dSq(float x, float y) const;
    float GetExactDist2d(float x, float y) const;
    float GetExactDist2dSq(Position const* pos) const;
    float GetExactDist2d(Position const* pos) const;
    float GetExactDistSq(float x, float y, float z) const;
    float GetExactDist(float x, float y, float z) const;
    float GetExactDistSq(Position const* pos) const;
    float GetExactDist(Position const* pos) const;

    void GetPositionOffsetTo(Position const& endPos, Position & retOffset) const;

    float GetAngle(Position const* pos) const;
    float GetAngle(float x, float y) const;
    float GetRelativeAngle(Position const* pos) const;
    float GetRelativeAngle(float x, float y) const;
    void GetSinCos(float x, float y, float &vsin, float &vcos) const;
    bool IsInDegreesRange(float x, float y, float degresA, float degresB, bool relative = false) const;
    float GetDegreesAngel(float x, float y, bool relative = false) const;

    Position GetRandPointBetween(Position const& B) const;
    void SimplePosXYRelocationByAngle(Position &pos, float dist, float angle, bool relative = false) const;

    bool IsInDist2d(float x, float y, float dist) const;
    bool IsInDist2d(Position const* pos, float dist) const;
    bool IsInDist(float x, float y, float z, float dist) const;
    bool IsInDist(Position const* pos, float dist) const;
    bool HasInArc(float arcangle, Position const* pos) const;
    bool HasInLine(WorldObject const* target, float width) const;
    bool IsWithinBox(Position const& center, float xradius, float yradius, float zradius) const;
    std::string ToString() const;

    Position operator-(Position const& rkVector) const;
    float magnitude() const;
    float length() const;

    bool IsLinesCross(Position const &pos11, Position const &pos12, Position const &pos21, Position const &pos22, Position * dest = nullptr) const;
    bool IsPointInBox(Position const& centerBox, std::vector<Position> box, Position const& point) const;
    void GeneratePointInBox(Position centerBox, std::vector<Position> box, std::vector<Position>& points, uint32 count = 1) const;
    void GenerateNonDuplicatePoints(std::list<Position>& randPosList, Position const& centerPos, uint8 maxPoint, float randMin, float randMax, float minDist) const;

    // modulos a radian orientation to the range of 0..2PI
    static float NormalizeOrientation(float o);

    static float NormalizePitch(float o);

    float m_positionX{};
    float m_positionY{};
    float m_positionZ{};
    float m_positionH{};
    float m_orientation{};
};

template<class Tag>
struct TaggedPosition
{
    TaggedPosition(float x = 0.0f, float y = 0.0f, float z = 0.0f, float o = 0.0f) : Pos(x, y, z, o) { }
    TaggedPosition(Position const& pos) : Pos(pos) { }

    TaggedPosition& operator=(Position const& pos)
    {
        Pos.Relocate(pos);
        return *this;
    }

    Position operator-(Position const& rkVector) const
    {
        return Position(Pos.m_positionX - rkVector.m_positionX, Pos.m_positionY - rkVector.m_positionY, Pos.m_positionZ - rkVector.m_positionZ);
    }

    operator Position() const { return Pos; }

    friend ByteBuffer& operator<<(ByteBuffer& buf, TaggedPosition const& tagged) { return buf << Position::ConstStreamer<Tag>(tagged.Pos); }
    friend ByteBuffer& operator >> (ByteBuffer& buf, TaggedPosition& tagged) { return buf >> Position::Streamer<Tag>(tagged.Pos); }

    Position Pos;
};

#define MAPID_INVALID 0xFFFFFFFF

class WorldLocation : public Position
{
public:
    explicit WorldLocation(uint32 _mapid = MAPID_INVALID, float _x = 0, float _y = 0, float _z = 0, float _o = 0);
    explicit WorldLocation(uint32 _mapid, Position const& pos);
    WorldLocation(WorldLocation const& loc);

    void WorldRelocate(WorldLocation const& loc);
    void WorldRelocate(uint32 _mapId = MAPID_INVALID, float _x = 0.f, float _y = 0.f, float _z = 0.f, float _o = 0.f);

    uint32 GetMapId() const;
    void SetMapId(uint32 _mapid);

    WorldLocation GetWorldLocation() const;

    uint32 m_mapId{};
};

ByteBuffer& operator<<(ByteBuffer& buf, Position::ConstStreamer<Position::XY> const& streamer);
ByteBuffer& operator>>(ByteBuffer& buf, Position::Streamer<Position::XY> const& streamer);
ByteBuffer& operator<<(ByteBuffer& buf, Position::ConstStreamer<Position::XYZ> const& streamer);
ByteBuffer& operator>>(ByteBuffer& buf, Position::Streamer<Position::XYZ> const& streamer);
ByteBuffer& operator<<(ByteBuffer& buf, Position::ConstStreamer<Position::XYZO> const& streamer);
ByteBuffer& operator>>(ByteBuffer& buf, Position::Streamer<Position::XYZO> const& streamer);
ByteBuffer& operator<<(ByteBuffer& buf, Position::ConstStreamer<Position::PackedXYZ> const& streamer);

#endif // Trinity_game_Position_h__
