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
		const Spline<int32>& spline = move_spline.spline;
		MoveSplineFlag splineflags = move_spline.splineflags;
		/*if (splineflags & MoveSplineFlag::UncompressedPath)
		{
		if (splineflags.cyclic)
		WriteCatmullRomCyclicPath(spline, data);
		else
		WriteCatmullRomPath(spline, data);
		}*/
		//else
		//WriteLinearPath(spline, data);
	}

    void PacketBuilder::WriteCreateBits(MoveSpline const& moveSpline, ByteBuffer& data)
    {
        if (!data.WriteBit(!moveSpline.Finalized()))
            return;

        data.WriteBit(moveSpline.splineflags & (MoveSplineFlag::Parabolic | MoveSplineFlag::Animation)); //   hasSplineStartTime = packet.ReadBit();
		data.WriteBit((moveSpline.splineflags & MoveSplineFlag::Parabolic) && moveSpline.effect_start_time < moveSpline.Duration());
		data.WriteBit(0); // unk
		data.WriteBits(moveSpline.getPath().size(), 25); // splineCount = packet.ReadBits("Spline Waypoints", 22, index);
        data.WriteBits(uint8(moveSpline.spline.mode()), 2); // packet.ReadEnum<SplineMode>("Spline Mode", 2, index);
        data.WriteBits(moveSpline.splineflags.raw(), 20); // packet.ReadEnum<SplineFlag434>("Spline flags", 20, index);
    }

    void PacketBuilder::WriteCreateData(MoveSpline const& moveSpline, ByteBuffer& data, Unit* unit)
    {
        if (!moveSpline.Finalized())
        {
            MoveSplineFlag splineFlags = moveSpline.splineflags;
            uint8 splineType = 0;

			data << moveSpline.timePassed();
			data << float(1.f);
			data << float(1.f);

			uint32 nodes = moveSpline.getPath().size();
			for (uint32 i = 0; i < nodes; ++i)
			{
				data << float(moveSpline.getPath()[i].x);
				data << float(moveSpline.getPath()[i].z);
				data << float(moveSpline.getPath()[i].y);
			}

			if (splineFlags & (MoveSplineFlag::Parabolic | MoveSplineFlag::Animation))
				data << moveSpline.effect_start_time;       // added in 3.1

			data << uint8(splineType);

			if (splineFlags.final_angle)
				data << moveSpline.facing.angle;

			if (splineFlags.final_point)
				data << moveSpline.facing.f.z << moveSpline.facing.f.y << moveSpline.facing.f.x;

            if ((splineFlags & MoveSplineFlag::Parabolic) && moveSpline.effect_start_time < moveSpline.Duration())
                data << moveSpline.vertical_acceleration;   // added in 3.1

            data << moveSpline.Duration();
        }

		data << float(unit->GetPositionX());
		data << float(unit->GetPositionZ());
        data << uint32(moveSpline.GetId());
        data << float(unit->GetPositionY());
    }

    void PacketBuilder::WriteCreateGuid(MoveSpline const& moveSpline, ByteBuffer& data)
    {
        if (!moveSpline.Finalized() && (moveSpline.splineflags & MoveSplineFlag::Mask_Final_Facing == MoveSplineFlag::Final_Target))
        {
            ObjectGuid facingGuid = moveSpline.facing.target;

            uint8 bitOrder[8] = { 4, 7, 0, 5, 1, 2, 3, 6 };
            data.WriteBitInOrder(facingGuid, bitOrder);

            uint8 byteOrder[8] = { 4, 2, 0, 5, 6, 3, 1, 7 };
            data.WriteBytesSeq(facingGuid, byteOrder);
        }
    }
}
