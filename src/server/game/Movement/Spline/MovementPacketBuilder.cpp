/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "MovementPacketBuilder.h"
#include "MoveSpline.h"
#include "WorldPacket.h"
#include "Object.h"

namespace Movement
{
    inline void operator << (ByteBuffer& b, const Vector3& v)
    {
        b << v.x << v.y << v.z;
    }

    inline void operator >> (ByteBuffer& b, Vector3& v)
    {
        b >> v.x >> v.y >> v.z;
    }

    enum MonsterMoveType
    {
        MonsterMoveNormal       = 0,
        MonsterMoveStop         = 1,
        MonsterMoveFacingSpot   = 2,
        MonsterMoveFacingTarget = 3,
        MonsterMoveFacingAngle  = 4
    };

	void WriteLinearPath(Spline<int32> const& spline, ByteBuffer& data)
	{
		uint32 last_idx = spline.getPointCount() - 3;
		Vector3 const* real_path = &spline.getPoint(1);

		if (last_idx > 0)
		{
			Vector3 middle = (real_path[0] + real_path[last_idx]) / 2.f;
			Vector3 offset;
			// first and last points already appended
			for (uint32 i = 0; i < last_idx; ++i)
			{
				offset = middle - real_path[i];
				data.appendPackXYZ(offset.x, offset.y, offset.z);
			}
		}
	}

	void WriteCatmullRomPath(const Spline<int32>& spline, ByteBuffer& data)
	{
		for (int i = 2; i < spline.getPointCount() - 3; i++)
			data << spline.getPoint(i).y << spline.getPoint(i).x << spline.getPoint(i).z;

		//data.append<Vector3>(&spline.getPoint(2), count);
	}

	void WriteCatmullRomCyclicPath(const Spline<int32>& spline, ByteBuffer& data)
	{
		data << spline.getPoint(1).y << spline.getPoint(1).x << spline.getPoint(1).z; // fake point, client will erase it from the spline after first cycle done

		for (int i = 1; i < spline.getPointCount() - 3; i++)
			data << spline.getPoint(i).y << spline.getPoint(i).x << spline.getPoint(i).z;
		//data.append<Vector3>(&spline.getPoint(1), count);
	}
	
    void PacketBuilder::WriteStopMovement(Vector3 const& pos, uint32 splineId, ByteBuffer& data)
	{
		ObjectGuid moverGUID;
		ObjectGuid transportGUID;

		data << float(pos.z);
		data << float(pos.x);
		data << uint32(splineId); //SplineId
		data << float(pos.y);
		data << float(0.f); // Most likely transport Y
		data << float(0.f); // Most likely transport Z
		data << float(0.f); // Most likely transport X

		data.WriteBit(1); // Parabolic speed // esi+4Ch
		data.WriteBit(moverGUID[0]);
		data.WriteBits(MonsterMoveStop, 3);
		data.WriteBit(1);
		data.WriteBit(1);
		data.WriteBit(1);
		data.WriteBits(0, 20);
		data.WriteBit(1);
		data.WriteBit(moverGUID[3]);
		data.WriteBit(1);
		data.WriteBit(1);
		data.WriteBit(1);
		data.WriteBit(1);
		data.WriteBit(moverGUID[7]);
		data.WriteBit(moverGUID[4]);
		data.WriteBit(1);
		data.WriteBit(moverGUID[5]);
		data.WriteBits(0, 22); // WP count
		data.WriteBit(moverGUID[6]);
		data.WriteBit(0); // Fake bit
		data.WriteBit(transportGUID[7]);
		data.WriteBit(transportGUID[1]);
		data.WriteBit(transportGUID[3]);
		data.WriteBit(transportGUID[0]);
		data.WriteBit(transportGUID[6]);
		data.WriteBit(transportGUID[4]);
		data.WriteBit(transportGUID[5]);
		data.WriteBit(transportGUID[2]);
		data.WriteBit(0); // Send no block
		data.WriteBit(0);
		data.WriteBit(moverGUID[2]);
		data.WriteBit(moverGUID[1]);

		data.FlushBits();

		data.WriteByteSeq(moverGUID[1]);

		data.WriteByteSeq(transportGUID[6]);
		data.WriteByteSeq(transportGUID[4]);
		data.WriteByteSeq(transportGUID[1]);
		data.WriteByteSeq(transportGUID[7]);
		data.WriteByteSeq(transportGUID[0]);
		data.WriteByteSeq(transportGUID[3]);
		data.WriteByteSeq(transportGUID[5]);
		data.WriteByteSeq(transportGUID[2]);

		data.WriteByteSeq(moverGUID[5]);
		data.WriteByteSeq(moverGUID[3]);
		data.WriteByteSeq(moverGUID[6]);
		data.WriteByteSeq(moverGUID[0]);
		data.WriteByteSeq(moverGUID[7]);
		data.WriteByteSeq(moverGUID[2]);
		data.WriteByteSeq(moverGUID[4]);
    }

	void PacketBuilder::WriteMonsterMove(const MoveSpline& move_spline, WorldPacket& data)
	{
		ObjectGuid moverGuid;
		ObjectGuid transportGuid;
		MoveSplineFlag splineflags = move_spline.splineflags;
		G3D::Vector3 const& firstPoint = move_spline.spline.getPoint(move_spline.spline.first());

		uint8 splineType = 0;

		switch (splineflags & MoveSplineFlag::Mask_Final_Facing)
		{
		case MoveSplineFlag::Final_Target:
			splineType = MonsterMoveFacingTarget;
			break;
		case MoveSplineFlag::Final_Angle:
			splineType = MonsterMoveFacingAngle;
			break;
		case MoveSplineFlag::Final_Point:
			splineType = MonsterMoveFacingSpot;
			break;
		default:
			splineType = MonsterMoveNormal;
			break;
		}

		data << float(firstPoint.z);
		data << float(firstPoint.x);
		data << uint32(move_spline.GetId());
		data << float(firstPoint.y);
		data << float(0.f); // Most likely transport Y
		data << float(0.f); // Most likely transport Z
		data << float(0.f); // Most likely transport X

		data.WriteBit(1); // Parabolic speed 21
		data.WriteBit(moverGuid[0]); // 32
		data.WriteBits(splineType, 3); // 68

		if (splineType == MonsterMoveFacingTarget)
		{
			ObjectGuid targetGuid = move_spline.facing.target;

			data.WriteBit(targetGuid[6]); // 190
			data.WriteBit(targetGuid[4]); // 188
			data.WriteBit(targetGuid[3]); // 187
			data.WriteBit(targetGuid[0]); // 184
			data.WriteBit(targetGuid[5]); // 189
			data.WriteBit(targetGuid[7]); // 191
			data.WriteBit(targetGuid[1]); // 185
			data.WriteBit(targetGuid[2]); // 186
		}

		data.WriteBit(1); // 19
		data.WriteBit(1); // 69
		data.WriteBit(1); // 120

		uint32 uncompressedSplineCount = move_spline.splineflags & MoveSplineFlag::UncompressedPath ? move_spline.splineflags.cyclic ? move_spline.spline.getPointCount() - 2 : move_spline.spline.getPointCount() - 3 : 1;
		data.WriteBits(uncompressedSplineCount, 20);

		data.WriteBit(!move_spline.splineflags.raw()); // 16
		data.WriteBit(moverGuid[3]); // 35
		data.WriteBit(1); // 108
		data.WriteBit(1); // 22
		data.WriteBit(1); // 109

		data.WriteBit(!move_spline.Duration()); // 20
		data.WriteBit(moverGuid[7]); // 39
		data.WriteBit(moverGuid[4]); // 36
		data.WriteBit(1); // 18
		data.WriteBit(moverGuid[5]); // 37

		int32 compressedSplineCount = move_spline.splineflags & MoveSplineFlag::UncompressedPath ? 0 : move_spline.spline.getPointCount() - 3;
		data.WriteBits(compressedSplineCount, 22); // WP count

		data.WriteBit(moverGuid[6]); // 38

		data.WriteBit(0); // unk

		data.WriteBit(transportGuid[7]); // 119
		data.WriteBit(transportGuid[1]); // 113
		data.WriteBit(transportGuid[3]); // 115
		data.WriteBit(transportGuid[0]); // 112
		data.WriteBit(transportGuid[6]); // 118
		data.WriteBit(transportGuid[4]); // 116
		data.WriteBit(transportGuid[5]); // 117
		data.WriteBit(transportGuid[2]); // 114

		data.WriteBit(0); // 176 unk
		data.WriteBit(0); // 56 unk

		data.WriteBit(moverGuid[2]); // 34
		data.WriteBit(moverGuid[1]); // 33

		data.FlushBits();

		if (compressedSplineCount)
			WriteLinearPath(move_spline.spline, data);

		data.WriteByteSeq(moverGuid[1]);

		data.WriteByteSeq(transportGuid[6]); // 118
		data.WriteByteSeq(transportGuid[4]); // 116
		data.WriteByteSeq(transportGuid[1]); // 113
		data.WriteByteSeq(transportGuid[7]); // 119
		data.WriteByteSeq(transportGuid[0]); // 112
		data.WriteByteSeq(transportGuid[3]); // 115
		data.WriteByteSeq(transportGuid[5]); // 117
		data.WriteByteSeq(transportGuid[2]); // 114

		if (splineflags & MoveSplineFlag::UncompressedPath)
		{
			if (splineflags.cyclic)
				WriteCatmullRomCyclicPath(move_spline.spline, data);
			else
				WriteCatmullRomPath(move_spline.spline, data);
		}
		else
		{
			G3D::Vector3 const& point = move_spline.spline.getPoint(move_spline.spline.getPointCount() - 2);
			data << point.y << point.x << point.z;
		}
		
		if (splineType == MonsterMoveFacingTarget)
		{
			ObjectGuid targetGuid = move_spline.facing.target;

			data.WriteByteSeq(targetGuid[5]); // 189
			data.WriteByteSeq(targetGuid[7]); // 191
			data.WriteByteSeq(targetGuid[0]); // 184
			data.WriteByteSeq(targetGuid[4]); // 188
			data.WriteByteSeq(targetGuid[3]); // 187
			data.WriteByteSeq(targetGuid[2]); // 186
			data.WriteByteSeq(targetGuid[6]); // 190
			data.WriteByteSeq(targetGuid[1]); // 185
		}

		data.WriteByteSeq(moverGuid[5]); // 37

		if (splineType == MonsterMoveFacingAngle)
			data << float(move_spline.facing.angle); // 45

		data.WriteByteSeq(moverGuid[3]); // 35

		data.WriteBit(0);                // 16

		if (move_spline.splineflags.raw())
			data << uint32(move_spline.splineflags.raw()); // 69

		data.WriteByteSeq(moverGuid[6]); // 38

		if (splineType == MonsterMoveFacingSpot)
			data << move_spline.facing.f.x << move_spline.facing.f.y << move_spline.facing.f.z;

		data.WriteByteSeq(moverGuid[0]); // 32
		data.WriteByteSeq(moverGuid[7]); // 39
		data.WriteByteSeq(moverGuid[2]); // 34
		data.WriteByteSeq(moverGuid[4]); // 36

		if (move_spline.Duration())
			data << uint32(move_spline.Duration());
    }

    void PacketBuilder::WriteCreateBits(MoveSpline const& moveSpline, ByteBuffer& data)
    {
        if (!data.WriteBit(!moveSpline.Finalized()))
            return;

        data.WriteBit(moveSpline.splineflags & (MoveSplineFlag::Parabolic | MoveSplineFlag::Animation)); //   hasSplineStartTime = packet.ReadBit();

        data.WriteBits(uint8(moveSpline.spline.mode()), 2); // packet.ReadEnum<SplineMode>("Spline Mode", 2, index);
        data.WriteBits(moveSpline.splineflags.raw(), 20); // packet.ReadEnum<SplineFlag434>("Spline flags", 20, index);
        data.WriteBit(0); // unk
        data.WriteBits(moveSpline.getPath().size(), 25); // splineCount = packet.ReadBits("Spline Waypoints", 22, index);
        data.WriteBit((moveSpline.splineflags & MoveSplineFlag::Parabolic) && moveSpline.effect_start_time < moveSpline.Duration());

        /*switch (moveSpline.splineflags & MoveSplineFlag::Mask_Final_Facing)
        {
            case MoveSplineFlag::Final_Target:
            {
                ObjectGuid targetGuid = moveSpline.facing.target;
                data.WriteBits(0, 2);
                data.WriteBit(targetGuid[4]);
                data.WriteBit(targetGuid[5]);
                data.WriteBit(targetGuid[0]);
                data.WriteBit(targetGuid[7]);
                data.WriteBit(targetGuid[1]);
                data.WriteBit(targetGuid[3]);
                data.WriteBit(targetGuid[2]);
                data.WriteBit(targetGuid[6]);
                break;
            }
            case MoveSplineFlag::Final_Angle:
                data.WriteBits(3, 2);
                break;
            case MoveSplineFlag::Final_Point:
                data.WriteBits(1, 2);
                break;
            default:
                data.WriteBits(2, 2);
                break;
        }*/
    }

    void PacketBuilder::WriteCreateData(MoveSpline const& moveSpline, ByteBuffer& data, Unit* unit)
    {
        if (!moveSpline.Finalized())
        {
            MoveSplineFlag splineFlags = moveSpline.splineflags;
            uint8 splineType = 0;

            if (splineFlags.final_point)
                splineType = 2;
            else if (splineFlags.final_angle)
                splineType = 4;
            else if ((splineFlags & MoveSplineFlag::Mask_Final_Facing) == MoveSplineFlag::Final_Target)
                splineType = MonsterMoveFacingTarget;

            data << uint8(splineType);
            data << float(1.f);                             // splineInfo.duration_mod; added in 3.1

            uint32 nodes = moveSpline.getPath().size();
            for (uint32 i = 0; i < nodes; ++i)
            {
                data << float(moveSpline.getPath()[i].z);
                data << float(moveSpline.getPath()[i].x);
                data << float(moveSpline.getPath()[i].y);
            }

            data << float(1.f);                             // splineInfo.duration_mod_next; added in 3.1

            if (splineFlags.final_point)
                data << moveSpline.facing.f.z << moveSpline.facing.f.y << moveSpline.facing.f.x;

            if (splineFlags.final_angle)
                data << moveSpline.facing.angle;

            if ((splineFlags & MoveSplineFlag::Parabolic) && moveSpline.effect_start_time < moveSpline.Duration())
                data << moveSpline.vertical_acceleration;   // added in 3.1

            if (splineFlags & (MoveSplineFlag::Parabolic | MoveSplineFlag::Animation))
                data << moveSpline.effect_start_time;       // added in 3.1

            data << moveSpline.timePassed();
            data << moveSpline.Duration();
        }

        data << uint32(moveSpline.GetId());
        data << float(unit->GetPositionZ());
        data << float(unit->GetPositionX());
        data << float(unit->GetPositionY());
    }

    void PacketBuilder::WriteCreateGuid(MoveSpline const& moveSpline, ByteBuffer& data)
    {
        if (!moveSpline.Finalized() && (moveSpline.splineflags & MoveSplineFlag::Mask_Final_Facing == MoveSplineFlag::Final_Target))
        {
            ObjectGuid facingGuid = moveSpline.facing.target;

            uint8 bitOrder[8] = { 3, 1, 0, 7, 6, 4, 5, 2 };
            data.WriteBitInOrder(facingGuid, bitOrder);

            uint8 byteOrder[8] = { 3, 0, 4, 6, 1, 5, 2, 7 };
            data.WriteBytesSeq(facingGuid, byteOrder);
        }
    }
}
