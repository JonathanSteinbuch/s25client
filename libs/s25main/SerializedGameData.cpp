// Copyright (c) 2005 - 2017 Settlers Freaks (sf-team at siedler25.org)
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

#include "SerializedGameData.h"
#include "CatapultStone.h"
#include "EventManager.h"
#include "FOWObjects.h"
#include "Game.h"
#include "GameEvent.h"
#include "GameObject.h"
#include "GamePlayer.h"
#include "RoadSegment.h"
#include "Ware.h"
#include "buildings/BurnedWarehouse.h"
#include "buildings/noBuildingSite.h"
#include "buildings/nobHQ.h"
#include "buildings/nobHarborBuilding.h"
#include "buildings/nobMilitary.h"
#include "buildings/nobShipYard.h"
#include "buildings/nobStorehouse.h"
#include "figures/nofAggressiveDefender.h"
#include "figures/nofArmorer.h"
#include "figures/nofAttacker.h"
#include "figures/nofBaker.h"
#include "figures/nofBrewer.h"
#include "figures/nofBuilder.h"
#include "figures/nofButcher.h"
#include "figures/nofCarpenter.h"
#include "figures/nofCarrier.h"
#include "figures/nofCatapultMan.h"
#include "figures/nofCharburner.h"
#include "figures/nofDefender.h"
#include "figures/nofDonkeybreeder.h"
#include "figures/nofFarmer.h"
#include "figures/nofFisher.h"
#include "figures/nofForester.h"
#include "figures/nofGeologist.h"
#include "figures/nofHunter.h"
#include "figures/nofIronfounder.h"
#include "figures/nofMetalworker.h"
#include "figures/nofMiller.h"
#include "figures/nofMiner.h"
#include "figures/nofMinter.h"
#include "figures/nofPassiveSoldier.h"
#include "figures/nofPassiveWorker.h"
#include "figures/nofPigbreeder.h"
#include "figures/nofPlaner.h"
#include "figures/nofScout_Free.h"
#include "figures/nofScout_LookoutTower.h"
#include "figures/nofShipWright.h"
#include "figures/nofStonemason.h"
#include "figures/nofTradeDonkey.h"
#include "figures/nofTradeLeader.h"
#include "figures/nofWarehouseWorker.h"
#include "figures/nofWellguy.h"
#include "figures/nofWoodcutter.h"
#include "helpers/containerUtils.h"
#include "helpers/format.hpp"
#include "helpers/toString.h"
#include "world/GameWorld.h"
#include "nodeObjs/noAnimal.h"
#include "nodeObjs/noCharburnerPile.h"
#include "nodeObjs/noDisappearingMapEnvObject.h"
#include "nodeObjs/noEnvObject.h"
#include "nodeObjs/noExtension.h"
#include "nodeObjs/noFighting.h"
#include "nodeObjs/noFire.h"
#include "nodeObjs/noFlag.h"
#include "nodeObjs/noGrainfield.h"
#include "nodeObjs/noGranite.h"
#include "nodeObjs/noShip.h"
#include "nodeObjs/noShipBuildingSite.h"
#include "nodeObjs/noSign.h"
#include "nodeObjs/noSkeleton.h"
#include "nodeObjs/noStaticObject.h"
#include "nodeObjs/noTree.h"
#include "s25util/Log.h"

// clang-format off
/// Version of the current game data
/// Usage: Always save for the most current version but include loading code that can cope with file format changes
/// If a format change occurred that can still be handled increase this version and handle it in the loading code.
/// If the change is to big to handle increase the version in Savegame.cpp and remove all code referencing GetGameDataVersion.
/// Then reset this number to 1.
/// Changelog:
/// 2: All player buildings together, variable width size for containers and ship names
/// 3: Landscape and terrain names stored as strings
/// 4: HunterWaitingForAnimalReady introduced as sub-state of HunterFindingShootingpoint
/// 5: Make RoadPathDirection contiguous and use optional for ware in nofBuildingWorker
/// 6: Make TradeDirection contiguous, Serialize only nobUsuals in BuildingRegister::buildings,
///    include water and fish in geologists resourceFound
static const unsigned currentGameDataVersion = 6;
// clang-format on

GameObject* SerializedGameData::Create_GameObject(const GO_Type got, const unsigned obj_id)
{
    switch(got)
    {
        case GO_Type::NobHq: return new nobHQ(*this, obj_id);
        case GO_Type::NobMilitary: return new nobMilitary(*this, obj_id);
        case GO_Type::NobStorehouse: return new nobStorehouse(*this, obj_id);
        case GO_Type::NobUsual: return new nobUsual(*this, obj_id);
        case GO_Type::NobShipyard: return new nobShipYard(*this, obj_id);
        case GO_Type::NobHarborbuilding: return new nobHarborBuilding(*this, obj_id);
        case GO_Type::NofAggressivedefender: return new nofAggressiveDefender(*this, obj_id);
        case GO_Type::NofAttacker: return new nofAttacker(*this, obj_id);
        case GO_Type::NofDefender: return new nofDefender(*this, obj_id);
        case GO_Type::NofPassivesoldier: return new nofPassiveSoldier(*this, obj_id);
        case GO_Type::NofPassiveworker: return new nofPassiveWorker(*this, obj_id);
        case GO_Type::NofWellguy: return new nofWellguy(*this, obj_id);
        case GO_Type::NofCarrier: return new nofCarrier(*this, obj_id);
        case GO_Type::NofWoodcutter: return new nofWoodcutter(*this, obj_id);
        case GO_Type::NofFisher: return new nofFisher(*this, obj_id);
        case GO_Type::NofForester: return new nofForester(*this, obj_id);
        case GO_Type::NofCarpenter: return new nofCarpenter(*this, obj_id);
        case GO_Type::NofStonemason: return new nofStonemason(*this, obj_id);
        case GO_Type::NofHunter: return new nofHunter(*this, obj_id);
        case GO_Type::NofFarmer: return new nofFarmer(*this, obj_id);
        case GO_Type::NofMiller: return new nofMiller(*this, obj_id);
        case GO_Type::NofBaker: return new nofBaker(*this, obj_id);
        case GO_Type::NofButcher: return new nofButcher(*this, obj_id);
        case GO_Type::NofMiner: return new nofMiner(*this, obj_id);
        case GO_Type::NofBrewer: return new nofBrewer(*this, obj_id);
        case GO_Type::NofPigbreeder: return new nofPigbreeder(*this, obj_id);
        case GO_Type::NofDonkeybreeder: return new nofDonkeybreeder(*this, obj_id);
        case GO_Type::NofIronfounder: return new nofIronfounder(*this, obj_id);
        case GO_Type::NofMinter: return new nofMinter(*this, obj_id);
        case GO_Type::NofMetalworker: return new nofMetalworker(*this, obj_id);
        case GO_Type::NofArmorer: return new nofArmorer(*this, obj_id);
        case GO_Type::NofBuilder: return new nofBuilder(*this, obj_id);
        case GO_Type::NofPlaner: return new nofPlaner(*this, obj_id);
        case GO_Type::NofGeologist: return new nofGeologist(*this, obj_id);
        case GO_Type::NofShipwright: return new nofShipWright(*this, obj_id);
        case GO_Type::NofScoutFree: return new nofScout_Free(*this, obj_id);
        case GO_Type::NofScoutLookouttower: return new nofScout_LookoutTower(*this, obj_id);
        case GO_Type::NofWarehouseworker: return new nofWarehouseWorker(*this, obj_id);
        case GO_Type::NofCatapultman: return new nofCatapultMan(*this, obj_id);
        case GO_Type::NofCharburner: return new nofCharburner(*this, obj_id);
        case GO_Type::NofTradedonkey: return new nofTradeDonkey(*this, obj_id);
        case GO_Type::NofTradeleader: return new nofTradeLeader(*this, obj_id);
        case GO_Type::Extension: return new noExtension(*this, obj_id);
        case GO_Type::Buildingsite: return new noBuildingSite(*this, obj_id);
        case GO_Type::Envobject: return new noEnvObject(*this, obj_id);
        case GO_Type::Fire: return new noFire(*this, obj_id);
        case GO_Type::Burnedwarehouse: return new BurnedWarehouse(*this, obj_id);
        case GO_Type::Flag: return new noFlag(*this, obj_id);
        case GO_Type::Grainfield: return new noGrainfield(*this, obj_id);
        case GO_Type::Granite: return new noGranite(*this, obj_id);
        case GO_Type::Sign: return new noSign(*this, obj_id);
        case GO_Type::Skeleton: return new noSkeleton(*this, obj_id);
        case GO_Type::Staticobject: return new noStaticObject(*this, obj_id);
        case GO_Type::Disappearingmapenvobject: return new noDisappearingMapEnvObject(*this, obj_id);
        case GO_Type::Tree: return new noTree(*this, obj_id);
        case GO_Type::Animal: return new noAnimal(*this, obj_id);
        case GO_Type::Fighting: return new noFighting(*this, obj_id);
        case GO_Type::Roadsegment: return new RoadSegment(*this, obj_id);
        case GO_Type::Ware: return new Ware(*this, obj_id);
        case GO_Type::Catapultstone: return new CatapultStone(*this, obj_id);
        case GO_Type::Ship: return new noShip(*this, obj_id);
        case GO_Type::Shipbuildingsite: return new noShipBuildingSite(*this, obj_id);
        case GO_Type::Charburnerpile: return new noCharburnerPile(*this, obj_id);
        case GO_Type::Economymodehandler: return new EconomyModeHandler(*this, obj_id);
        case GO_Type::Nothing:
        case GO_Type::Unknown: RTTR_Assert(false); break;
    }
    throw Error("Invalid GameObjectType " + helpers::toString(got) + " for objId=" + helpers::toString(obj_id)
                + " found!");
}

FOWObject* SerializedGameData::Create_FOWObject(const FoW_Type fowtype)
{
    switch(fowtype)
    {
        default: return nullptr;
        case FoW_Type::Building: return new fowBuilding(*this);
        case FoW_Type::Buildingsite: return new fowBuildingSite(*this);
        case FoW_Type::Flag: return new fowFlag(*this);
        case FoW_Type::Tree: return new fowTree(*this);
        case FoW_Type::Granite: return new fowGranite(*this);
    }
}

SerializedGameData::SerializedGameData()
    : debugMode(false), gameDataVersion(0), expectedNumObjects(0), em(nullptr), writeEm(nullptr), isReading(false)
{}

void SerializedGameData::Prepare(bool reading)
{
    static const std::array<char, 4> versionID = {"VER"};
    if(reading)
    {
        std::array<char, 4> versionIDRead;
        PopRawData(&versionIDRead.front(), versionIDRead.size());
        if(versionIDRead != versionID)
            throw Error("Invalid file format!");
        gameDataVersion = PopUnsignedInt();
    } else
    {
        Clear();
        PushRawData(&versionID.front(), versionID.size());
        PushUnsignedInt(currentGameDataVersion);
        gameDataVersion = currentGameDataVersion;
    }
    writtenObjIds.clear();
    readObjects.clear();
    expectedNumObjects = 0;
    isReading = reading;
}

void SerializedGameData::MakeSnapshot(const std::shared_ptr<Game>& game)
{
    Prepare(false);

    GameWorld& gw = game->world_;
    writeEm = &gw.GetEvMgr();

    // Anzahl Objekte reinschreiben (used for safety checks only)
    expectedNumObjects = GameObject::GetNumObjs();
    PushUnsignedInt(expectedNumObjects);

    // World and objects
    gw.Serialize(*this);
    // EventManager
    writeEm->Serialize(*this);
    if(game->ggs_.objective == GameObjective::EconomyMode)
    {
        PushObject(gw.econHandler.get(), true);
    }
    // Spieler serialisieren
    for(unsigned i = 0; i < gw.GetNumPlayers(); ++i)
    {
        if(debugMode)
            LOG.write("Start serializing player %1% at %2%\n") % i % GetLength();
        gw.GetPlayer(i).Serialize(*this);
        if(debugMode)
            LOG.write("Done serializing player %1% at %2%\n") % i % GetLength();
    }

    static boost::format evCtError("Event count mismatch. Expected: %1%, written: %2%");
    static boost::format objCtError("Object count mismatch. Expected: %1%, written: %2%");

    if(writtenEventIds.size() != writeEm->GetNumActiveEvents())
        throw Error((evCtError % writeEm->GetNumActiveEvents() % writtenEventIds.size()).str());
    // If this check fails, we missed some objects or some objects were destroyed without decreasing the obj count
    if(expectedNumObjects != writtenObjIds.size() + 1) // "Nothing" nodeObj does not get serialized
        throw Error((objCtError % expectedNumObjects % (writtenObjIds.size() + 1)).str());

    writeEm = nullptr;
    writtenObjIds.clear();
    writtenEventIds.clear();
}

void SerializedGameData::ReadSnapshot(const std::shared_ptr<Game>& game, ILocalGameState& localGameState)
{
    Prepare(true);

    GameWorld& gw = game->world_;
    em = &gw.GetEvMgr();

    expectedNumObjects = PopUnsignedInt();

    gw.Deserialize(game, localGameState, *this);
    em->Deserialize(*this);
    if(game->ggs_.objective == GameObjective::EconomyMode)
    {
        gw.econHandler.reset(PopObject<EconomyModeHandler>(GO_Type::Economymodehandler));
    }

    for(unsigned i = 0; i < gw.GetNumPlayers(); ++i)
        gw.GetPlayer(i).Deserialize(*this);

    static boost::format evCtError("Event count mismatch. Expected: %1%, read: %2%");
    static boost::format objCtError("Object count mismatch. Expected: %1%, Existing: %2%");
    static boost::format objCtError2("Object count mismatch. Expected: %1%, read: %2%");

    // If this check fails, we did not serialize all objects or there was an async
    if(readEvents.size() != em->GetNumActiveEvents())
        throw Error((evCtError % em->GetNumActiveEvents() % readEvents.size()).str());
    if(expectedNumObjects != GameObject::GetNumObjs())
        throw Error((objCtError % expectedNumObjects % GameObject::GetNumObjs()).str());
    if(expectedNumObjects != readObjects.size() + 1) // "Nothing" nodeObj does not get serialized
        throw Error((objCtError2 % expectedNumObjects % (readObjects.size() + 1)).str());

    em = nullptr;
    readObjects.clear();
    readEvents.clear();
}

void SerializedGameData::PushObject_(const GameObject* go, const bool known)
{
    RTTR_Assert(!isReading);

    // Gibts das Objekt gar nich?
    if(!go)
    {
        // Null draufschreiben
        PushUnsignedInt(0);
        return;
    }

    const unsigned objId = go->GetObjId();

    RTTR_Assert(objId <= GameObject::GetObjIDCounter());
    if(objId > GameObject::GetObjIDCounter())
    {
        LOG.write("%s\n") % _("An error occured while saving which was suppressed!");
        PushUnsignedInt(0);
        return;
    }

    PushUnsignedInt(objId);

    // If the object was already serialized skip the data
    if(IsObjectSerialized(objId))
    {
        if(debugMode)
            LOG.write("Saved known objId %u\n") % objId;
        return;
    }

    if(debugMode)
        LOG.write("Saving objId %u, obj#=%u\n") % objId % writtenObjIds.size();

    // Objekt merken
    writtenObjIds.insert(objId);

    RTTR_Assert(writtenObjIds.size() < GameObject::GetNumObjs());

    // Objekt nich bekannt? Dann Type-ID noch mit drauf
    if(!known)
        PushEnum<uint16_t>(go->GetGOT());

    // Objekt serialisieren
    if(debugMode)
        LOG.write("Start serializing %1% at %2%\n") % objId % GetLength();
    go->Serialize(*this);
    if(debugMode)
        LOG.write("Done serializing %1% at %2%\n") % objId % GetLength();

    // Sicherheitscode reinschreiben
    PushUnsignedShort(GetSafetyCode(*go));
}

void SerializedGameData::PushEvent(const GameEvent* event)
{
    if(!event)
    {
        PushUnsignedInt(0);
        return;
    }

    unsigned instanceId = event->GetInstanceId();
    PushUnsignedInt(instanceId);
    if(IsEventSerialized(instanceId))
        return;
    writtenEventIds.insert(instanceId);
    if(debugMode)
        LOG.write("Start serializing event %1% at %2%\n") % instanceId % GetLength();
    event->Serialize(*this);
    if(debugMode)
        LOG.write("Done serializing event %1% at %2%\n") % instanceId % GetLength();
    PushUnsignedShort(GetSafetyCode(*event));
}

const GameEvent* SerializedGameData::PopEvent()
{
    unsigned instanceId = PopUnsignedInt();
    if(!instanceId)
        return nullptr;

    // Note: em->GetEventInstanceCtr() might not be set yet
    const auto foundObj = readEvents.find(instanceId);
    if(foundObj != readEvents.end())
        return foundObj->second;
    std::unique_ptr<GameEvent> ev = std::make_unique<GameEvent>(*this, instanceId);

    unsigned short safety_code = PopUnsignedShort();

    if(safety_code != GetSafetyCode(*ev))
    {
        LOG.write("SerializedGameData::PopEvent: ERROR: After loading Event(instanceId = %1%); Code is wrong!\n")
          % instanceId;
        throw Error("Invalid safety code after PopEvent");
    }
    return ev.release();
}

/// FoW-Objekt
void SerializedGameData::PushFOWObject(const FOWObject* fowobj)
{
    // Gibts das Objekt gar nich?
    if(!fowobj)
    {
        // Null draufschreiben
        PushUnsignedChar(0);
        return;
    }

    // Objekt-Typ
    PushEnum<uint8_t>(fowobj->GetType());

    // Objekt serialisieren
    fowobj->Serialize(*this);
}

FOWObject* SerializedGameData::PopFOWObject()
{
    // Typ auslesen
    auto type = Pop<FoW_Type>();

    // Kein Objekt?
    if(type == FoW_Type::Nothing)
        return nullptr;

    // entsprechendes Objekt erzeugen
    return Create_FOWObject(type);
}

GameObject* SerializedGameData::PopObject_(GO_Type got)
{
    RTTR_Assert(isReading);
    // Obj-ID holen
    const unsigned objId = PopUnsignedInt();

    // Obj-ID = 0 ? Dann Null-Pointer zurueckgeben
    if(!objId)
        return nullptr;

    GameObject* go = GetReadGameObject(objId);

    // Schon vorhanden?
    if(go)
        // dann das nehmen
        return go;

    // Objekt nich bekannt? Dann in den heiligen Schriften lesen
    if(got == GO_Type::Unknown)
        got = Pop<GO_Type>();

    // und erzeugen
    go = Create_GameObject(got, objId);

    // Sicherheitscode auslesen
    unsigned short safety_code = PopUnsignedShort();

    if(safety_code != GetSafetyCode(*go))
    {
        LOG.write(
          "SerializedGameData::PopObject_: ERROR: After loading Object(obj_id = %u, got = %u); Code is wrong!\n")
          % objId % rttr::enum_cast(got);
        delete go;
        throw Error("Invalid safety code after PopObject");
    }

    return go;
}

unsigned short SerializedGameData::GetSafetyCode(const GameObject& go)
{
    return 0xFFFF ^ rttr::enum_cast(go.GetGOT()) ^ go.GetObjId();
}

unsigned short SerializedGameData::GetSafetyCode(const GameEvent& ev)
{
    return 0xFFFF ^ ev.GetInstanceId();
}

SerializedGameData::Error SerializedGameData::makeOutOfRange(unsigned value, unsigned maxValue)
{
    return Error(helpers::format("%s is out of range. Maximum allowed value: %s", value, maxValue));
}

void SerializedGameData::AddObject(GameObject* go)
{
    RTTR_Assert(isReading);
    RTTR_Assert(!readObjects[go->GetObjId()]); // Do not call this multiple times per GameObject
    readObjects[go->GetObjId()] = go;
    RTTR_Assert(readObjects.size() < expectedNumObjects);
}

unsigned SerializedGameData::AddEvent(unsigned instanceId, GameEvent* ev)
{
    RTTR_Assert(isReading);
    RTTR_Assert(!readEvents[instanceId]); // Do not call this multiple times per GameObject
    readEvents[instanceId] = ev;
    return instanceId;
}

bool SerializedGameData::IsObjectSerialized(unsigned obj_id) const
{
    RTTR_Assert(!isReading);
    RTTR_Assert(obj_id <= GameObject::GetObjIDCounter());
    return helpers::contains(writtenObjIds, obj_id);
}

bool SerializedGameData::IsEventSerialized(unsigned evInstanceid) const
{
    RTTR_Assert(!isReading);
    RTTR_Assert(evInstanceid < writeEm->GetEventInstanceCtr());
    return helpers::contains(writtenEventIds, evInstanceid);
}

GameObject* SerializedGameData::GetReadGameObject(const unsigned obj_id) const
{
    RTTR_Assert(isReading);
    RTTR_Assert(obj_id <= GameObject::GetObjIDCounter());
    auto foundObj = readObjects.find(obj_id);
    if(foundObj == readObjects.end())
        return nullptr;
    else
        return foundObj->second;
}
