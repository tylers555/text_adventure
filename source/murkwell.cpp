internal ta_id
ChooseRandomTARoom(ta_system *TA){
    u32 RandomIndex = GetRandomNumber(267982) % TA->RoomTable.BucketsUsed;
    for(u32 I=0; I<TA->RoomTable.MaxBuckets; I++){
        if(!TA->RoomTable.Hashes[I]) continue;
        if(I == RandomIndex){
            return TA->RoomTable.Keys[I];
        }
    }
    return MakeTAID(0);
}

//~ Ghost

// NOTE(Tyler): This is a debug thing in case a room has not been defined yet in the SJA
internal void
GhostEnsureRoom(ta_system *TA, entity_ghost *Ghost){
    while(!HashTableFindPtr(&TA->RoomTable, Ghost->CurrentRoom)){
        LogMessage("Room does not exist!");
        Ghost->CurrentRoom = ChooseRandomTARoom(TA);
    }
}

internal void
GameAddGhost(ta_system *TA, asset_system *Assets, ta_id Room=MakeTAID(0)){
    entity_ghost *Ghost = ArrayAlloc(&TA->Ghosts);
    Ghost->Item = TAItemByName(TA, "Albert (Ghost)");
    Ghost->CurrentRoom = Room.ID ? Room : ChooseRandomTARoom(TA);
    GhostEnsureRoom(TA, Ghost);
    TAFindRoom(TA, Ghost->CurrentRoom)->Ghost = Ghost->Item;
}

internal void
GameRemoveGhost(ta_system *TA, u32 Index){
    entity_ghost *Ghost = &TA->Ghosts[Index];
    TAFindRoom(TA, Ghost->CurrentRoom)->Ghost = MakeTAID(0);
    ArrayUnorderedRemove(&TA->Ghosts, Index);
}

internal void
GameRemoveAllGhosts(ta_system *TA){
    for(u32 I=0; I<TA->Ghosts.Count; I++){
        entity_ghost *Ghost = &TA->Ghosts[I];
        TARoomRemoveItemByID(TA, TAFindRoom(TA, Ghost->CurrentRoom), Ghost->Item);
    }
    ArrayClear(&TA->Ghosts);
}

internal void
GhostChangeRoom(ta_system *TA, asset_system *Assets, entity_ghost *Ghost, ta_id NewRoom){
    TAFindRoom(TA, Ghost->CurrentRoom)->Ghost = MakeTAID(0);
    Ghost->CurrentRoom = NewRoom;
    GhostEnsureRoom(TA, Ghost);
    TAFindRoom(TA, Ghost->CurrentRoom)->Ghost = Ghost->Item;
}

internal void
GameUpdateGhosts(ta_system *TA, asset_system *Assets){
    for(u32 I=0; I<TA->Ghosts.Count; I++){
        entity_ghost *Ghost = &TA->Ghosts[I];
        ta_room *Room = HashTableFindPtr(&TA->RoomTable, Ghost->CurrentRoom);
        
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

internal b8
GhostOverrideDescription(game_renderer *Renderer, ta_system *TA, asset_system *Assets, 
                         console_theme *Theme, asset_font *Font, rect *RoomDescriptionRect,
                         ta_room *Room){
    if(Room->Ghost.ID){
        ta_item *Ghost = TAFindItem(TA, Room->Ghost);
        ta_data *GhostDescription = TAFindDescription(&Ghost->Datas, AssetTag(AssetTag_Ghost));
        if(GhostDescription){
            DoString(Renderer, Font, Theme->DescriptionFancies, ArrayCount(Theme->DescriptionFancies),
                     GhostDescription->Data, RoomDescriptionRect);
        }
        return true;
    }
    
    if(Room->Flags & RoomFlag_HadGhost){
        
    }
    
    return false;
}

//~ 
internal void
GameProcessItem(ta_system *TA, ta_id ItemID, ta_item *Item){
    if(HasTag(Item->Tag, AssetTag_Haunted)){
        ArrayAdd(&TA->HauntedItems, ItemID);
    }
}

internal void
GameTick(ta_system *TA, asset_system *Assets){
    ArenaClear(&GlobalTickMemory);
    GameUpdateGhosts(TA, Assets);
}
