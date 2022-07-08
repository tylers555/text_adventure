
//~ @movement_commands
b8 HelperCommandGoTo(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, word_array Words){
    ta_room *FoundRoom = 0;
    u32 FoundIndex = 0;
    ta_name_comparison Found = MakeTANameComparison();
    b8 IsAmbiguous = false;
    
    FOR_RANGE(I, 0, Direction_TOTAL){
        ta_room *Room = HashTableFindPtr(&TA->RoomTable, TA->CurrentRoom->Adjacents[I]);
        if(!Room) continue;
        
        ta_name_comparison Comparison = TACompareWordsAndName(&Room->NameData, Words);
        
        if(Comparison.FoundWordIndex >= 0){
            if(Found.FoundWordIndex >= 0){
                // TODO(Tyler): I'm not quite sure what the behavior here ought to be yet.
                if(Found.FoundWordIndex >= Comparison.FoundWordIndex){
                    if(Comparison.Weight > Found.Weight){
                        FoundRoom = Room;
                        FoundIndex = I;
                        Found = Comparison;
                    }else if(Comparison.Weight == Found.Weight){
                        IsAmbiguous = true;
                        break;
                    }
                }else{
                    IsAmbiguous = true;
                    break;
                }
            }else{
                FoundRoom = Room;
                FoundIndex = I;
                Found = Comparison;
            }
        }
    }
    
    if(IsAmbiguous){
        TA->Respond(GetVar(Assets, goto_specific));
        TA->Callback = HelperCommandGoTo;
        return true;
    }
    
    if(FoundRoom){
        asset_tag *Tag = &TA->CurrentRoom->AdjacentTags[FoundIndex];
        if(HasTag(*Tag, AssetTag_Locked)){
            if(TA->AttemptToUnlock(Mixer, Assets, TA->CurrentRoom, Tag)){
                TA->Respond(GetVar(Assets, auto_unlock));
            }else{
                TA->Respond(GetVar(Assets, locked));
                return true;
            }
        }
        
        TA->CurrentRoom = FoundRoom;
        Mixer->PlaySound(GetSoundEffect(Assets, AssetID(sound_move)));
        
        return true;
    }
    
    return false;
}

b8 CommandMove(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, word_array Words){
    ta_room *CurrentRoom = TA->CurrentRoom;
    ta_room *NextRoom = 0;
    
    FOR_EACH(Word, &Words){
        direction Direction = Direction_None;
        
        f32 HighestMatch = 0.0f;
#define DIRECTION(Name, D) { f32 Match = CompareWordsPercentage(Word, Name); \
if(Match > HighestMatch) { \
Direction = D; \
HighestMatch = Match; \
} \
}
        DIRECTIONS
#undef DIRECTIONS
        
        if(MaybeDoHauntedMove(TA, Assets, Direction)) return true;
        
        if(HighestMatch <= WORD_MATCH_THRESHOLD){
            continue;
        }
        
        ta_id NextRoomString = CurrentRoom->Adjacents[Direction];
        NextRoom = HashTableFindPtr(&TA->RoomTable, NextRoomString);
        if(!NextRoom){
            TA->Respond(GetVar(Assets, move_invalid_room));
            return false;
        }
        
        asset_tag *Tag = &CurrentRoom->AdjacentTags[Direction];
        if(HasTag(*Tag, AssetTag_Locked)){
            if(TA->AttemptToUnlock(Mixer, Assets, CurrentRoom, Tag)){
                TA->Respond(GetVar(Assets, auto_unlock));
                break;
            }else{
                TA->Respond(GetVar(Assets, locked));
                return false;
            }
        }else if(TA->IsClosed(*Tag)){
            return false;
        }
        
        break;
    }
    
    if(NextRoom){
        TA->CurrentRoom = NextRoom;
        Mixer->PlaySound(GetSoundEffect(Assets, AssetID(sound_move)));
        
        return true;
    }
    
    if(HelperCommandGoTo(Mixer, TA, Assets, Words)){
        return true;
    }
    
    TA->Respond(GetVar(Assets, move_invalid_all));
    return false;
}

b8 CommandExit(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, word_array Words){
    ta_room *Room = TA->CurrentRoom;
    ta_data *Data = TA->FindData(&Room->Datas, TADataType_Room, AssetTag(AssetTag_Exit));
    if(!Data){
        // TODO(Tyler): Callback!
        TA->Respond(GetVar(Assets, exit_where));
        return false;
    }
    
    for(u32 I=0; I<Direction_TOTAL; I++){
        if(Room->Adjacents[I] == Data->TAID){
            asset_tag *Tag = &Room->AdjacentTags[I];
            if(HasTag(*Tag, AssetTag_Locked)){
                if(TA->AttemptToUnlock(Mixer, Assets, Room, Tag)){
                    TA->Respond(GetVar(Assets, auto_unlock));
                    break;
                }else{
                    TA->Respond(GetVar(Assets, locked));
                    return false;
                }
            }else if(TA->IsClosed(*Tag)){
                return false;
            }
        }
    }
    
    ta_room *NextRoom = HashTableFindPtr(&TA->RoomTable, Data->TAID);
    if(!NextRoom){
        LogMessage("That room does not exist!");
        TA->Respond("ERROR!");
        return false;
    }
    
    TA->CurrentRoom = NextRoom;
    Mixer->PlaySound(GetSoundEffect(Assets, AssetID(sound_move)));
    
    return true;
}

// TODO(Tyler): This ought to take a room to go to in some cases
b8 CommandEnter(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, word_array Words){
    // TODO(Tyler): I'm not sure if there is a better way to do this.
    if(Words.Count > 1){
        return HelperCommandGoTo(Mixer, TA, Assets, Words);
    }
    
    ta_room *Room = TA->CurrentRoom;
    ta_data *Data = TA->FindData(&Room->Datas, TADataType_Room, AssetTag(AssetTag_Enter));
    if(!Data){
        TA->Respond(GetVar(Assets, enter_where));
        return false;
    }
    
    for(u32 I=0; I<Direction_TOTAL; I++){
        if(Room->Adjacents[I] == Data->TAID){
            asset_tag *Tag = &Room->AdjacentTags[I];
            if(HasTag(*Tag, AssetTag_Locked)){
                if(TA->AttemptToUnlock(Mixer, Assets, Room, Tag)){
                    TA->Respond(GetVar(Assets, auto_unlock));
                    break;
                }else{
                    TA->Respond(GetVar(Assets, locked));
                    return false;
                }
            }else if(TA->IsClosed(*Tag)){
                return false;
            }
        }
    }
    
    ta_room *NextRoom = HashTableFindPtr(&TA->RoomTable, Data->TAID);
    if(NextRoom){
        LogMessage("That room does not exist!");
        TA->Respond("ERROR!");
        return false;
    }
    
    TA->CurrentRoom = NextRoom;
    Mixer->PlaySound(GetSoundEffect(Assets, AssetID(sound_move)));
    
    return true;
}

//~ @item_commands

b8 CallbackConfirmBuy(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, word_array Words){
    ta_room *Room = TA->CurrentRoom;
    ta_item *Item = HashTableFindPtr(&TA->ItemTable, Room->Items[TA->BuyItemIndex]);
    if(!Item){
        LogMessage("Item does not exist!");
        TA->Respond("ERROR!");
        return false;
    }
    
    f32 PositiveWeight = 0.0f;
    f32 NegativeWeight = 0.0f;
    for(u32 I=0; I<Words.Count; I++){
        const char *Word = Words[I];
#define WORD(Name) { f32 M = CompareWordsPercentage(Word, Name); if(M > PositiveWeight) PositiveWeight = M; }
        POSITIVES
#undef WORD
#define WORD(Name) { f32 M = CompareWordsPercentage(Word, Name); if(M > NegativeWeight) NegativeWeight = M; }
        NEGATIVES
#undef WORD
    }
    
    if(Maximum(PositiveWeight, NegativeWeight) <= WORD_MATCH_THRESHOLD){
        TA->Respond(GetVar(Assets, buy_callback_prompt), Item->NameData.Name);
        TA->Callback = CallbackConfirmBuy;
    }else if(PositiveWeight > NegativeWeight){
        if(TA->InventoryAddItem(Room->Items[TA->BuyItemIndex])) TA->RoomRemoveItem(Room, TA->BuyItemIndex);
        else return false;
        TA->Money -= Item->Cost;
        Item->IsDirty = true;
        Item->Cost = 0;
        
        Mixer->PlaySound(GetSoundEffect(Assets, AssetID(sound_item_bought)));
        
        ta_data *Description = TA->FindDescription(&Item->Datas, AssetTag(AssetTag_Examine));
        if(Description) TA->Respond(Description->Data);
    }else{
        TA->Respond(GetVar(Assets, buy_callback_no));
    }
    
    return true;
}

b8 CommandTake(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, word_array Words){
    ta_room *Room = TA->CurrentRoom;
    
    ta_found_items FoundItems = TAFindItems(TA, &Room->Items, Words);
    HANDLE_FOUND_ITEMS(FoundItems, CommandTake, "take");
    
    u32 RemovedItems = 0;
    for(u32 I=0; I<FoundItems.Items.Count; I++){
        ta_found_item *FoundItem = &FoundItems.Items[I];
        
        ta_item *Item = FoundItem->Item;
        u32 Index = FoundItem->ItemIndex;
        
        if(HasTag(Item->Tag, AssetTag_Static)){
            ta_data *Description = TA->FindDescription(&Item->Datas, AssetTag(AssetTag_Take));
            if(Description){
                TA->Respond("%s", Description->Data);
                continue;
            }
        }else if(Item->Cost > 0){
            TA->Respond(GetVar(Assets, take_buy));
            TA->Respond(GetVar(Assets, take_buy_prompt));
            TA->Callback = CallbackConfirmBuy;
            TA->BuyItemIndex = Index-RemovedItems;
            break;
        }
        
        if(TA->InventoryAddItem(Room->Items[Index-RemovedItems])) TA->RoomRemoveItem(Room, Index-RemovedItems);
        else return false;
        RemovedItems++;
        
        ta_data *Description = TA->FindDescription(&Item->Datas, AssetTag(AssetTag_Examine));
        if(!Description) return false;
        TA->Respond(Description->Data);
        Mixer->PlaySound(GetSoundEffect(Assets, AssetID(sound_item_taken)));
    }
    
    return true;
}

b8 CommandDrop(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, word_array Words){
    ta_room *Room = TA->CurrentRoom;
    
    ta_found_items FoundItems = TAFindItems(TA, &TA->Inventory, Words);
    HANDLE_FOUND_ITEMS(FoundItems, CommandDrop, "drop");
    
    u32 RemovedItems = 0;
    for(u32 I=0; I<FoundItems.Items.Count; I++){
        ta_found_item *FoundItem = &FoundItems.Items[I];
        
        ta_item *Item = FoundItem->Item;
        u32 Index = FoundItem->ItemIndex;
        
        if(TA->RoomDropItem(Assets, Room, TA->Inventory[Index-RemovedItems])) TA->InventoryRemoveItem(Index-RemovedItems);
        RemovedItems++;
        Mixer->PlaySound(GetSoundEffect(Assets, AssetID(sound_item_dropped)));
    }
    
    return true;
}

b8 CommandBuy(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, word_array Words){
    ta_room *Room = TA->CurrentRoom;
    
    ta_found_items FoundItems = TAFindItems(TA, &Room->Items, Words);
    HANDLE_FOUND_ITEMS(FoundItems, CommandBuy, "buy");
    
    u32 RemovedItems = 0;
    for(u32 I=0; I<FoundItems.Items.Count; I++){
        ta_found_item *FoundItem = &FoundItems.Items[I];
        
        ta_item *Item = FoundItem->Item;
        u32 Index = FoundItem->ItemIndex;
        
        if(HasTag(Item->Tag, AssetTag_Static)){
            TA->Respond(GetVar(Assets, buy_static));
            continue;
        }
        
        if(Item->Cost == 0){
            TA->Respond(GetVar(Assets, buy_free));
            if(TA->InventoryAddItem(Room->Items[Index-RemovedItems])) TA->RoomRemoveItem(Room, Index-RemovedItems);
            else return false;
            RemovedItems++;
            
            ta_data *Description = TA->FindDescription(&Item->Datas, AssetTag(AssetTag_Examine));
            if(!Description) continue;
            TA->Respond(Description->Data);
            Mixer->PlaySound(GetSoundEffect(Assets, AssetID(sound_item_taken)));
        }else if(TA->Money >= Item->Cost){
            TA->Respond(GetVar(Assets, buy_prompt));
            TA->Callback = CallbackConfirmBuy;
            TA->BuyItemIndex = Index-RemovedItems;
            break;
        }else{
            TA->Respond(GetVar(Assets, buy_too_poor));
            continue;
        }
    }
    
    return true;
}

b8 CommandEat(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, word_array Words){
    ta_found_items FoundItems = TAFindItems(TA, &TA->Inventory, Words);
    HANDLE_FOUND_ITEMS(FoundItems, CommandEat, "eat");
    
    u32 RemovedItems = 0;
    for(u32 I=0; I<FoundItems.Items.Count; I++){
        ta_found_item *FoundItem = &FoundItems.Items[I];
        
        ta_item *Item = FoundItem->Item;
        u32 Index = FoundItem->ItemIndex;
        
        ta_data *Description = TA->FindDescription(&Item->Datas, AssetTag(AssetTag_Eat));
        if(!Description){
            TA->Respond(GetVar(Assets, eat_cant));
            continue;
        }
        TA->Respond(Description->Data);
        
        if(HasTag(Item->Tag, AssetTag_Bread)){
            TA->InventoryAddItem(GetItemID(TA, bread_crumbs));
        }
        
        TA->InventoryRemoveItem(Index-RemovedItems);
        RemovedItems++;
    }
    
    if(RemovedItems > 0){
        Mixer->PlaySound(GetSoundEffect(Assets, AssetID(sound_item_eaten)));
        return true;
    }else{
        TA->Respond(GetVar(Assets, eat_invalid)); 
        TA->Callback = CommandEat;
    }
    return false;
}

b8 CommandPlay(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, word_array Words){
    ta_room *Room = TA->CurrentRoom;
    
    ta_found_items FoundItems = TAFindItems(TA, &TA->Inventory, Words);
    TAContinueFindItems(TA, &Room->Items, Words, &FoundItems);
    HANDLE_FOUND_ITEMS(FoundItems, CommandPlay, "play");
    
    b8 DidPlay = false;
    for(u32 I=0; I<FoundItems.Items.Count; I++){
        ta_found_item *FoundItem = &FoundItems.Items[I];
        
        ta_item *Item = FoundItem->Item;
        
        asset_tag_id Extra = AssetTag_None;
        if(HasTag(Item->Tag, AssetTag_Organ)) Extra = TA->OrganState;
        
        ta_data *Sound = TA->FindData(&Item->Datas, TADataType_Asset, AssetTag(AssetTag_Sound, AssetTag_Play, Extra));
        if(Sound) Mixer->PlaySound(GetSoundEffect(Assets, Sound->Asset));
        
        ta_data *Description = TA->FindDescription(&Item->Datas, AssetTag(AssetTag_Play, Extra));
        if(!Description) continue;
        TA->Respond(Description->Data);
        DidPlay = true;
    }
    
    if(DidPlay) return true;
    
    for(u32 I=0; I<Room->Items.Count; I++){
        ta_item *Item = TA->FindItem(Room->Items[I]);
        ta_data *Description = TA->FindDescription(&Item->Datas, AssetTag(AssetTag_Play));
        if(Description){
            asset_tag_id Extra = AssetTag_None;
            if(HasTag(Item->Tag, AssetTag_Organ)) Extra = TA->OrganState;
            ta_data *Sound = TA->FindData(&Item->Datas, TADataType_Asset, AssetTag(AssetTag_Sound, AssetTag_Play, Extra));
            if(Sound) Mixer->PlaySound(GetSoundEffect(Assets, Sound->Asset));
            
            TA->Respond(Description->Data);
            return true;
        }
    }
    
    return false;
}

b8 CommandExamine(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, word_array Words){
    ta_room *Room = TA->CurrentRoom;
    
    ta_found_items FoundItems = TAFindItems(TA, &TA->Inventory, Words);
    TAContinueFindItems(TA, &Room->Items, Words, &FoundItems);
    HANDLE_FOUND_ITEMS(FoundItems, CommandExamine, "examine");
    
    for(u32 I=0; I<FoundItems.Items.Count; I++){
        ta_found_item *FoundItem = &FoundItems.Items[I];
        
        ta_item *Item = FoundItem->Item;
        
        asset_tag Tag = AssetTag(AssetTag_Examine);
        if(HasTag(Item->Tag, AssetTag_Broken)){
            Tag = AssetTag(AssetTag_Examine, AssetTag_Broken);
        }else if(HasTag(Item->Tag, AssetTag_Repaired)){
            Tag = AssetTag(AssetTag_Examine, AssetTag_Repaired);
        }
        
        ta_data *Description = TA->FindDescription(&Item->Datas, Tag);
        if(!Description) continue;
        TA->Respond(Description->Data);
    }
    
    if(FoundItems.Items.Count == 0){
        if(Words.Count > 1){
            TA->Respond(GetVar(Assets, examine_invalid));
        }else{
            TA->Respond(GetVar(Assets, examine_none));
            TA->Callback = CommandExamine;
        }
        
        return false;
    }
    
    return true;
}

b8 CommandUnlock(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, word_array Words){
    ta_room *CurrentRoom = TA->CurrentRoom;
    for(u32 I=0; I<Direction_TOTAL; I++){
        asset_tag Tag = CurrentRoom->AdjacentTags[I];
        if(!HasTag(Tag, AssetTag_Locked)) continue;
        if(TA->AttemptToUnlock(Mixer, Assets, CurrentRoom, &Tag)){
            TA->Respond(GetVar(Assets, auto_unlock));
        }else{
            TA->Respond(GetVar(Assets, locked));
        }
    }
    
    return true;
}

b8 CommandWait(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, word_array Words){
    Assert(0);
    TA->Respond("Not yet implemented!");
    return true;
}

b8 CommandRepair(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, word_array Words){
    ta_room *Room = TA->CurrentRoom;
    
    ta_found_items FoundItems =  TAFindItems(TA, &TA->Inventory, Words);
    TAContinueFindItems(TA, &Room->Items, Words, &FoundItems);
    
    ta_item *Item = 0;
    b8 FixedSomething = false;
    
    FOR_EACH_PTR(FoundItem, &FoundItems.Items){
        Item = FoundItem->Item;
        
        if(!HasTag(Item->Tag, AssetTag_Broken)){
            continue;
        }
        
        ta_data *Data = TA->FindData(&Item->Datas, TADataType_Item, AssetTag(AssetTag_Fixer));
        if(!Data){
            TA->Respond(GetVar(Assets, repair_cant));
            continue;
        }
        
        FOR_EACH_(FixerItemID, J, &TA->Inventory){
            if(FixerItemID != Data->TAID) continue;
            
            SwitchTag(&Item->Tag, AssetTag_Broken, AssetTag_Repaired);
            TA->InventoryRemoveItem(J);
            ta_data *Sound = TA->FindData(&Item->Datas, TADataType_Asset, AssetTag(AssetTag_Sound, AssetTag_Repaired));
            TAUpdateOrganState(TA);
            if(!Sound) break;
            Mixer->PlaySound(GetSoundEffect(Assets, Sound->Asset));
            break;
        }
        
        FixedSomething = true;
    }
    
    if(!FixedSomething && Item && !HasTag(Item->Tag, AssetTag_Broken)) TA->Respond(GetVar(Assets, repair_not_broken));
    
    return true;
}

b8 CommandUse(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, word_array Words){
    ta_room *Room = TA->CurrentRoom;
    
    ta_found_items FoundItems =  TAFindItems(TA, &TA->Inventory, Words);
    //TAContinueFindItems(TA, &Room->Items, Words, &FoundItems);
    
    for(u32 I=0; I<FoundItems.Items.Count; I++){
        ta_found_item *FoundItem = &FoundItems.Items[I];
        ta_item *Item = FoundItem->Item;
        
        ta_data *Data = TA->FindData(&Item->Datas, TADataType_Command, AssetTag(AssetTag_Use));
        if(!Data){
            TA->Respond(GetVar(Assets, use_dont_know));
            continue;
        }
        
        word_array Tokens = TokenizeCommand(&GlobalTransientMemory, Data->Data);
        TA->DispatchCommand(Mixer, Assets, Tokens);
        
        return true;
    }
    
    TA->Respond(GetVar(Assets, use_dont_have));
    return false;
}

b8 CommandRead(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, word_array Words){
    ta_room *Room = TA->CurrentRoom;
    
    ta_found_items FoundItems =  TAFindItems(TA, &TA->Inventory, Words);
    TAContinueFindItems(TA, &Room->Items, Words, &FoundItems);
    HANDLE_FOUND_ITEMS(FoundItems, CommandRead, "read");
    
    for(u32 I=0; I<FoundItems.Items.Count; I++){
        ta_found_item *FoundItem = &FoundItems.Items[I];
        
        ta_item *Item = FoundItem->Item;
        
        asset_tag Tag = AssetTag(AssetTag_Read);
        
        ta_data *Description = TA->FindDescription(&Item->Datas, Tag);
        if(!Description) continue;
        TA->Respond(Description->Data);
    }
    
    if(FoundItems.Items.Count == 0){
        if(Words.Count > 1){
            TA->Respond(GetVar(Assets, examine_invalid));
        }else{
            TA->Respond(GetVar(Assets, examine_none));
            TA->Callback = CommandExamine;
        }
        
        return false;
    }
    
    return true;
}

//~ @game_commands
b8 CommandPray(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, word_array Words){
    ta_room *Room = TA->CurrentRoom;
    
    if(TA->CarillonPages.QuestStatus == QuestStatus_Active){
        if(Room == GetRoom(TA, bench)){
            TA->Respond(GetVar(Assets, pray_carillon_pages_bench));
            return true;
        }
    }
    
    TA->Respond(GetVar(Assets, pray_response));
    // TODO(Tyler): Sound effect
    return true;
}

b8 CommandPerformRitual(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, word_array Words){
    ta_room *Room = TA->CurrentRoom;
    
    if(TA->CarillonPages.QuestStatus != QuestStatus_Active){
        TA->Respond(GetVar(Assets, perform_ritual_carillon_pages_inactive));
        return false;
    }
    if(Room != GetRoom(TA, bench)){
        TA->Respond(GetVar(Assets, perform_ritual_carillon_pages_wrong_room));
        return false;
    }
    if(!TA->InventoryHasItem(GetItemID(TA, ritual_book))){
        TA->Respond(GetVar(Assets, perform_ritual_carillon_pages_no_book));
        return false;
    }
    if(!TA->RoomHasItem(Room, GetItemID(TA, ritual_candles))){
        TA->Respond(GetVar(Assets, perform_ritual_carillon_pages_no_candles));
        return false;
    }
    
    ta_id Page = GetItemID(TA, carillon_page_4);
    if(!TA->InventoryAddItem(Page)) Assert(TA->RoomAddItem(Room, Page));
    
    TA->Respond(GetVar(Assets, perform_ritual_carillon_pages_successful));
    Mixer->PlaySound(GetSoundEffect(Assets, AssetID(sound_perform_ritual)));
    TA->RoomRemoveItemByID(Room, GetItemID(TA, carillon_pages_bench_ghost));
    return true;
}

b8 CommandPerform(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, word_array Words){
    ta_room *Room = TA->CurrentRoom;
    
    ta_found_items FoundItems = TAFindItems(TA, &TA->Inventory, Words);
    TAContinueFindItems(TA, &Room->Items, Words, &FoundItems);
    
    b8 FoundSomething = false;
    FOR_EACH_PTR(FoundItem, &FoundItems.Items){
        if(HasTag(FoundItem->Item->Tag, AssetTag_Ritual)){
            return CommandPerformRitual(Mixer, TA, Assets, Words); 
        }
        FoundSomething = true;
    }
    
    if(!FoundSomething) FOR_EACH(ItemID, &Room->Items){
        ta_item *Item = TA->FindItem(ItemID);
        if(Item == GetItem(TA, bench_carillon_pages_ritual)){
            return CommandPerformRitual(Mixer, TA, Assets, Words);
        }
    }
    
    return true;
}

b8 CommandSetupCandles(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, word_array Words){
    ta_room *Room = TA->CurrentRoom;
    
    ta_found_items FoundItems = TAFindItems(TA, &TA->Inventory, Words);
    
    ta_id Candles = GetItemID(TA, ritual_candles);
    ta_id RitualBook = GetItemID(TA, ritual_book);
    FOR_EACH_PTR(FoundItem, &FoundItems.Items){
        ta_item *Item = FoundItem->Item;
        
        if((Item->ID == Candles) || HasTag(Item->Tag, AssetTag_Ritual)){
            if(TA->InventoryHasItem(Candles) && TA->InventoryHasItem(RitualBook)){
                if(TA->RoomDropItem(Assets, Room, Candles)){
                    Mixer->PlaySound(GetSoundEffect(Assets, AssetID(sound_setup_candles)));
                    TA->Respond(GetVar(Assets, setup_candles_carillon_pages));
                    TA->InventoryRemoveItemByID(Candles);
                    return true;
                }
            }
        }
    }
    
    if(FoundItems.Items.Count == 0){
        if(TA->InventoryHasItem(Candles) && TA->InventoryHasItem(RitualBook)){
            if(TA->RoomDropItem(Assets, Room, Candles)){
                Mixer->PlaySound(GetSoundEffect(Assets, AssetID(sound_setup_candles)));
                TA->Respond(GetVar(Assets, setup_candles_carillon_pages));
                TA->InventoryRemoveItemByID(Candles);
                return true;
            }
        }
    }
    
    TA->Respond(GetVar(Assets, setup_candles_cant));
    return false;
}

b8 CommandFeed(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, word_array Words){
    ta_room *Room = TA->CurrentRoom;
    ta_found_items FoundItems = TAFindItems(TA, &TA->Inventory, Words);
    
    ta_item *Eater = 0;
    ta_item *Food = 0;
    FOR_EACH_PTR(FoundItem, &FoundItems.Items){
        ta_item *Item = FoundItem->Item;
        if(HasTag(Item->Tag, AssetTag_Food)){
            Food = Item;
            continue;
        }
        if(TAItemIsAnEater(TA, Item)){
            Eater = Item;
            continue;
        }
    }
    
    b8 IsAmbiguous = false;
    if(!Food) FOR_EACH(ItemID, &TA->Inventory){
        ta_item *Item = TA->FindItem(ItemID);
        if(HasTag(Item->Tag, AssetTag_Food)){
            if(Food) IsAmbiguous = true;
            Food = Item;
        }
    }
    
    if(!Eater && !IsAmbiguous) FOR_EACH(ItemID, &Room->Items){
        ta_item *Item = TA->FindItem(ItemID);
        if(TAItemIsAnEater(TA, Item)){
            if(Eater) IsAmbiguous = true;
            Eater = Item;
        }
    }
    
    if(!Eater){
        TA->Respond(GetVar(Assets, feed_no_eater));
        return false;
    }
    
    if(!Food){
        TA->Respond(GetVar(Assets, feed_no_food));
        return false;
    }
    
    if(IsAmbiguous){
        TA->Respond(GetVar(Assets, ambiguous_items)); 
        TA->Callback = CommandFeed; 
        return false; 
    }
    
    TA->InventoryRemoveItemByID(Food->ID);
    if((Eater->ID == GetItemID(TA, carillon_pages_bench_ghost)) &&
       (Food->ID == GetItemID(TA, apple_pie))){
        TA->CarillonPages.BenchGhostIsFollowing = true;
        ta_id Page = GetItemID(TA, carillon_page_4);
        if(!TA->InventoryAddItem(Page)) Assert(TA->RoomAddItem(Room, Page));
        TA->Respond(GetVar(Assets, feed_carillon_pages_bench_ghost_pie));
    }else{
        ta_data *Feed = TA->FindDescription(&Eater->Datas, AssetTag(AssetTag_Feed));
        TA->Respond(Feed->Data);
    }
    
    Mixer->PlaySound(GetSoundEffect(Assets, AssetID(sound_item_fed)));
    
    return true;
}

//~ @testing_commands
b8 CommandTestAddMoney(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, word_array Words){
    TA->Money += 10;
    return true;
}

b8 CommandTestSubMoney(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, word_array Words){
    if(TA->Money >= 10) TA->Money -= 10;
    return true;
}

b8 CommandTestCarillonPages(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, word_array Words){
    MurkwellStartCarillonPages(TA, Assets);
    return true;
}

//~ @meta_commands
b8 CommandMusic(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, word_array Words){
    if(!SettingsState.MusicHandle.ID){
        SettingsState.MusicHandle = Mixer->PlaySound(GetSoundEffect(Assets, AssetID(sound_test_music)), MixerSoundFlag_Loop);
    }else{
        Mixer->StopSound(SettingsState.MusicHandle);
        SettingsState.MusicHandle = {};
    }
    
    return true;
}

b8 CommandUndo(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, word_array Words){
    TA->Respond(GetVar(Assets, undo_message));
    return true;
}

b8 CommandRedo(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, word_array Words){
    TA->Respond(GetVar(Assets, redo_message));
    return true;
}
