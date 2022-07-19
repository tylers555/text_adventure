
//~ Helpers
internal asset_id
ChooseRandomTARoom(ta_system *TA){
    NOT_IMPLEMENTED_YET;
#if 0
    u32 RandomIndex = GetRandomNumber(267982) % TA->RoomTable.BucketsUsed;
    for(u32 I=0; I<TA->RoomTable.MaxBuckets; I++){
        if(!TA->RoomTable.Hashes[I]) continue;
        if(I == RandomIndex){
            return TA->RoomTable.Keys[I];
        }
    }
#endif
    return {};
}

internal b8
MurkwellPostEvent(ta_system *TA, murkwell_event_type Type){
    if(TA->Event.Type) return false;
    TA->Event.Type = Type;
    return true;
}

//~ @entity_ghost

// NOTE(Tyler): This is a debug thing in case a room has not been defined yet in the SJA
internal void
GhostEnsureRoom(ta_system *TA, entity_ghost *Ghost){
    while(!TA->FindRoom(Ghost->CurrentRoom)){
        LogMessage("Room does not exist!");
        Ghost->CurrentRoom = ChooseRandomTARoom(TA);
    }
}

internal void
MurkwellAddGhost(ta_system *TA, asset_system *Assets, asset_id Room=MakeAssetID(0)){
    entity_ghost *Ghost = ArrayAlloc(&TA->Ghosts);
    Ghost->Item = AssetID(Item, ghost_albert);
    Ghost->CurrentRoom = Room.ID ? Room : ChooseRandomTARoom(TA);
    GhostEnsureRoom(TA, Ghost);
    TA->FindRoom(Ghost->CurrentRoom)->Ghost = Ghost->Item;
}

internal void
MurkwellRemoveGhost(ta_system *TA, u32 Index){
    entity_ghost *Ghost = &TA->Ghosts[Index];
    TA->FindRoom(Ghost->CurrentRoom)->Ghost = MakeAssetID(0);
    ArrayUnorderedRemove(&TA->Ghosts, Index);
}

internal void
MurkwellRemoveAllGhosts(ta_system *TA){
    for(u32 I=0; I<TA->Ghosts.Count; I++){
        entity_ghost *Ghost = &TA->Ghosts[I];
        TA->RoomRemoveItemByID(TA->FindRoom(Ghost->CurrentRoom), Ghost->Item);
    }
    ArrayClear(&TA->Ghosts);
}

internal void
GhostChangeRoom(ta_system *TA, asset_system *Assets, entity_ghost *Ghost, asset_id NewRoom){
    TA->FindRoom(Ghost->CurrentRoom)->Ghost = MakeAssetID(0);
    Ghost->CurrentRoom = NewRoom;
    GhostEnsureRoom(TA, Ghost);
    TA->FindRoom(Ghost->CurrentRoom)->Ghost = Ghost->Item;
}

internal void
MurkwellUpdateGhosts(ta_system *TA, asset_system *Assets){
    for(u32 I=0; I<TA->Ghosts.Count; I++){
        entity_ghost *Ghost = &TA->Ghosts[I];
        ta_room *Room = TA->FindRoom(Ghost->CurrentRoom);
        
        b8 DoDrop = (GetRandomNumber(5142) % 10) < 1; // 10% chance of droppping
        
        b8 DoMove = (GetRandomNumber(132578) % 10) < 4; // 40% chance of moving
        if(!DoMove) continue;
        
        u32 AdjacentCount = 0;
        for(u32 J=0; J<Direction_TOTAL; J++){
            if(Room->Adjacents[J].ID) AdjacentCount++;
        }
        
        if(AdjacentCount == 0) continue;
        direction Direction = Direction_None;
        u32 MoveIndex = (GetRandomNumber(9325718) % AdjacentCount);
        u32 Index = 0;
        for(u32 J=0; J<Direction_TOTAL; J++){
            if(Room->Adjacents[J].ID){
                if(Index == MoveIndex){
                    Direction = (direction)J;
                    break;
                }
                Index++;
            }
        }
        Assert(Direction);
        
        GhostChangeRoom(TA, Assets, Ghost, Room->Adjacents[Direction]);
    }
}

internal b8
IsRoomHaunted(ta_system *TA, ta_room *Room){
    for(u32 I=0; I<TA->Ghosts.Count; I++){
        entity_ghost *Ghost = &TA->Ghosts[I];
        for(u32 J=0; J<Room->Items.Count; J++){
            if(Room->Items[J] == Ghost->Item) return true;
        }
    }
    
    return false;
}

internal b8
MaybeDoHauntedMove(ta_system *TA, asset_system *Assets, direction Direction){
    ta_room *Room = TA->CurrentRoom;
    if(!IsRoomHaunted(TA, Room)) return false;
    
    return true;
}

internal ta_data *
GhostOverrideDescription(ta_system *TA, asset_system *Assets, ta_room *Room){
    if(Room->Ghost.ID){
        ta_item *Ghost = TA->FindItem(Room->Ghost);
        ta_data *GhostDescription = TA->FindDescription(&Ghost->Datas, AssetTag(AssetTag_Ghost));
        if(GhostDescription) return GhostDescription;
    }
    
    return 0;
}

//~ @quest_organ
internal inline b8
MurkwellIsOrganQuestDone(ta_system *TA){
    b8 Result = (TA->OrganState == AssetTag_Repaired);
    return Result;
}

//~ @quest_carillon_pages

internal b8
MurkwellStartCarillonPages(ta_system *TA, asset_system *Assets){
    // TODO(Tyler): The probability to go up with more moves and depending on whether or not the 
    // organ quest is done.
    
    //MurkwellPostEvent(TA, MurkwellEvent_CarillonPages);
    TA->CarillonPages.QuestStatus = QuestStatus_Active;
    TA->CarillonPages.OrganWasRepaired = MurkwellIsOrganQuestDone(TA);
    if(!TA->CarillonPages.OrganWasRepaired){
        TA->RoomEnsureItem(TAFind(TA, Room, cathedral_quire), AssetID(Item, carillon_pages_singing_ghost));
    }
    
    TA->RoomEnsureItem(TAFind(TA, Room, bench), AssetID(Item, carillon_pages_bench_ghost));
    
    return true;
}

//~ 
// TODO(Tyler): This isn't really used right now
internal void
MurkwellProcessItem(ta_system *TA, asset_id ItemID, ta_item *Item){
    if(HasTag(Item->Tag, AssetTag_Haunted)){
        ArrayAdd(&TA->HauntedItems, ItemID);
    }
}

internal const char *
MurkwellOverrideRoomDescription(ta_system *TA, asset_system *Assets, ta_room *Room){
    ta_data *Data;
    if((Data = GhostOverrideDescription(TA, Assets, Room)) != 0) return Data->Data;
    
    if(TA->Event.Type){
        return GetVar(Assets, event_carillon_pages);
    }
    
    if(HasTag(Room->Tag, AssetTag_Organ)){
        if((Data = TA->FindDescription(&Room->Datas, AssetTag(TA->OrganState))) != 0) return Data->Data;
    }else if(HasTag(Room->Tag, AssetTag_CarillonPages)){
        if((Data = TA->FindDescription(&Room->Datas, AssetTag(AssetTag_CarillonPages))) != 0) return Data->Data;
    }
    
    return 0;
}

internal const char *
MurkwellAdditionalRoomDescription(ta_system *TA, asset_system *Assets, ta_room *Room){
    if(TA->CarillonPages.BenchGhostIsFollowing){
        return GetVar(Assets, carillon_pages_bench_ghost_following);
    }else if(TA->RoomHasItem(Room, AssetID(Item, carillon_pages_ghostly_congregation))){
        ta_data *Data = TA->FindDescription(&TAFind(TA, Item, carillon_pages_ghostly_congregation)->Datas, 
                                            AssetTag(AssetTag_Override));
        if(Data) return Data->Data;
    }
    
    return 0;
}

internal void
MurkwellTick(ta_system *TA, asset_system *Assets){
    TA->Event = {};
    
    MurkwellUpdateGhosts(TA, Assets);
    
    //~ Carillon pages
    
    if(((TA->CurrentRoom == TAFind(TA, Room, plaza_southeast)) ||
        (TA->CurrentRoom == TAFind(TA, Room, plaza_southwest)) ||
        (TA->CurrentRoom == TAFind(TA, Room, plaza_northwest)) ||
        (TA->CurrentRoom == TAFind(TA, Room, james_street))) &&
       ((GetRandomNumber(73219) % 10) < 1)){
        MurkwellStartCarillonPages(TA, Assets);
    }
}
