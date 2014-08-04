/*
* Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
* Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

//#include "OnlineShopMgr.h"
#include "Opcodes.h"
#include "DatabaseEnv.h"
#include "WorldSession.h"

void WorldSession::HandleOnlineShopQueryOpcode(WorldPacket& recvData)
{
	sLog->outDebug(LOG_FILTER_NETWORKIO, "WORLD: Received CMSG_ONLINE_SHOP_QUERY");

	SendOnlineShopQueryResult();
}

void WorldSession::SendOnlineShopQueryResult()
{
	WorldPacket data(SMSG_ONLINE_SHOP_QUERY_RESULT);
	/*
	if (v4 + 4)
	{
		data << uint8(0);  // 40
		
		for (uint8 i = 0; i < 24; i++)
		{
			data << uint8(0);  // 5172
			data << uint8(0);  // 5184
			data << uint8(0);  // 5168
				
			if (5168 == 0)
			{
				data << uint32(1); // 36
				data << uint32(1); // 1062
				data << uint8(0);  // 24
				data << uint8(0);  // 16
				data << uint8(0);  // 32
				data << uint8(0);  // 5164
				data << uint32(1); // 549
			}
			data << uint8(0);  // 5180

			if (5180)
			{
				data << uint32(1); // 5176
			}
			++i;
		} 
		while (i < 24);
		
		data << uint8(0);  // 5204
		
		if (5204)
			{
				data << uint8(0);  // 52
				data << uint32(1); // 72
				data << uint32(1); // 1098
				data << uint32(1); // 585
				data << uint8(0);  // 60
				data << uint8(0);  // 5200
				data << uint8(0);  // 68
			}
			++i;
		}
		while (i < 4);
	}

	if (v4 + 9)
	{
		data << uint8(0);  // 5180
		
		if (5180 == 0)
			{
				data << uint32(1); // 48
				data << uint8(0);  // 5176
				data << uint32(1); // 561
				data << uint32(1); // 1074
				data << uint8(0);  // 28
				data << uint8(0);  // 44
				data << uint8(0);  // 36
			}
			++v57;
		} 
	while (v57 < *((_DWORD *)v4 + 9));
	}

	if (v4 + 14))
	{
		++v58;
		*(_DWORD *)(v18 + 4) = v19;
	} while (v58 < *((_DWORD *)v4 + 14));

	if (v4 + 14))
	{
		do
		{
			data << uint32(1); // 268
			data << uint8(0);  // 4
			data << uint8(0);  // 264
			data << uint32(1); // 260
			++v57;
			data << uint32(1); // 15
		} while (v57 < *((_DWORD *)v4 + 14));
		v17 = 0;
	}

	if (v4 + 4)
	{
		data << uint8(0);  // 41

			if (v4 + 5) + 24))
			{
				if (5168)
				{
					if (16)
					{
						data << uint32(1); // 12
					}
					
					if (*(_BYTE *)(*(_DWORD *)(*((_DWORD *)v4 + 5) + v17 + 28) + v24 + 32))
					{
						data << uint32(1); // 28
					}
					
					if (*(_BYTE *)(*(_DWORD *)(*((_DWORD *)v4 + 5) + v17 + 28) + v24 + 5164))
					{
						data << uint32(1); // 5160
					}
					
					data << uint8(0);  // 36
					
					if (((v4 + 5) + v17 + 28) + v24 + 24)
					{
						data << uint32(1); // 20
					}
					
					data << uint8(0);  // 549
					data << uint8(0);  // 1062
				}
				
				data << uint32(1); // 4
				data << uint32(1); // 8
				++v57;
				data << uint32(1); // 5188
			} 
			while (v57 < v4 + 5) + v17 + 24));
	}
	
	if (v30 + v17 + 5204)
	{
		if (v30 + v17 + 52)
		{
			data << uint32(1); // 48
		}
		
		if ((v4 + 5) + v17 + 68)
		{
			data << uint32(1); // 64
		}
		
		data << uint8(0);  // 585
		data << uint8(0);  // 72
		
		if (*(_BYTE *)(*((_DWORD *)v4 + 5) + v17 + 5200))
		{
			data << uint32(1); // 5196
		}
		
		if ((v4 + 5) + v17 + 60)
		{
			data << uint32(1); // 56
		}
		
		data << uint8(0);  // 1098
	}
	
	data << uint32(1); // 5
	data << uint32(1); // 44
	data << uint32(1); // 8
	data << uint32(1); // 12
	++v58;
	data << uint32(1); // 16
	data << uint32(1); // 20
} 
while ((unsigned int)v36 < *((_DWORD *)v4 + 4));

if (v4 + 9) >(unsigned int)v17)
{
	if (v38 + v17 + 5180)
	{
		if (v38 + v17 + 5176)
		{
			data << uint32(1); // 5172
		}
		
		data << uint8(0);  // 561
		
		if ((v4 + 10) + v17 + 36)
		{
			data << uint32(1); // 32
		}
		
		if ((v4 + 10) + v17 + 44)
		{
			data << uint32(1); // 40
		}
		
		if ((v4 + 10) + v17 + 28)
		{
			data << uint32(1); // 24
		}
		
		data << uint8(0);  // 1074
		data << uint8(0);  // 48
	}
	
	data << uint32(1); // 16
	data << uint8(0);  // 20
	data << uint32(1); // 10
	data << uint32(1); // 12
	data << uint32(1); // 4
	++v57;
	data << uint32(1); // 8
} 
while (v57 < v4 + 9));

}
data << uint32(1); // 1
return result;
*/
}