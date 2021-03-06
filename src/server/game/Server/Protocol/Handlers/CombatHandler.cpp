/*
 * Copyright (C) 2008 - 2011 Trinity <http://www.trinitycore.org/>
 *
 * Copyright (C) 2010 - 2012 Myth Project <http://mythprojectnetwork.blogspot.com/>
 *
 * Myth Project's source is based on the Trinity Project source, you can find the
 * link to that easily in Trinity Copyrights. Myth Project is a private community.
 * To get access, you either have to donate or pass a developer test.
 * You can't share Myth Project's sources! Only for personal use.
 */

#include "Common.h"
#include "Log.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "ObjectAccessor.h"
#include "CreatureAI.h"
#include "ObjectDefines.h"

void WorldSession::HandleAttackSwingOpcode(WorldPacket & recv_data)
{
    uint64 guid;
    recv_data >> guid;

    sLog->outStaticDebug("WORLD: Recvd CMSG_ATTACKSWING Message guidlow:%u guidhigh:%u", GUID_LOPART(guid), GUID_HIPART(guid));

    Unit* pEnemy = ObjectAccessor::GetUnit(*_player, guid);

    if(!pEnemy)
    {
        if(!IS_UNIT_GUID(guid))
            sLog->outError("WORLD: Object %u (TypeID: %u) isn't player, pet or creature", GUID_LOPART(guid), GuidHigh2TypeId(GUID_HIPART(guid)));
        else
            sLog->outError("WORLD: Enemy %s %u not found", GetLogNameForGuid(guid), GUID_LOPART(guid));

        // stop attack state at client
        SendAttackStop(NULL);
        return;
    }

    if(!_player->canAttack(pEnemy))
    {
        // stop attack state at client
        SendAttackStop(pEnemy);
        return;
    }

    _player->Attack(pEnemy, true);
}

void WorldSession::HandleAttackStopOpcode(WorldPacket & /*recv_data*/)
{
    GetPlayer()->AttackStop();
}

void WorldSession::HandleSetSheathedOpcode(WorldPacket & recv_data)
{
    uint32 sheathed;
    recv_data >> sheathed;

    //sLog->outDebug(LOG_FILTER_PACKETIO, "WORLD: Recvd CMSG_SETSHEATHED Message guidlow:%u value1:%u", GetPlayer()->GetGUIDLow(), sheathed);

    if(sheathed >= MAX_SHEATH_STATE)
    {
        sLog->outError("Unknown sheath state %u ??", sheathed);
        return;
    }

    GetPlayer()->SetSheath(SheathState(sheathed));
}

void WorldSession::SendAttackStop(Unit const* enemy)
{
    WorldPacket data(SMSG_ATTACKSTOP, (8+8+4));             // we guess size
    data.append(GetPlayer()->GetPackGUID());
    data.append(enemy ? enemy->GetPackGUID() : 0);          // must be packed guid
    data << uint32(0);                                      // unk, can be 1 also
    SendPacket(&data);
}

