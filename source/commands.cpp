
//~ Movement commands
b8 HelperCommandGoTo(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    ta_room *FoundRoom = 0;
    u32 FoundIndex = 0;
    ta_name_comparison Found = MakeTANameComparison();
    b8 IsAmbiguous = false;
    
    for(u32 I=0; I < Direction_TOTAL; I++){
        ta_room *Room = HashTableFindPtr(&TA->RoomTable, TA->CurrentRoom->Adjacents[I]);
        if(!Room) continue;
        
        ta_name_comparison Comparison = TACompareWordsAndName(&Room->NameData, Words, WordCount);
        
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
            if(TAAttemptToUnlock(Mixer, TA, Assets, TA->CurrentRoom, Tag)){
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

b8 CommandMove(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    ta_room *CurrentRoom = TA->CurrentRoom;
    ta_room *NextRoom = 0;
    
    for(u32 I=0; I < WordCount; I++){
        char *Word = Words[I];
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
            if(TAAttemptToUnlock(Mixer, TA, Assets, CurrentRoom, Tag)){
                TA->Respond(GetVar(Assets, auto_unlock));
                break;
            }else{
                TA->Respond(GetVar(Assets, locked));
                return false;
            }
        }else if(TAIsClosed(TA, *Tag)){
            return false;
        }
        
        break;
    }
    
    if(NextRoom){
        TA->CurrentRoom = NextRoom;
        Mixer->PlaySound(GetSoundEffect(Assets, AssetID(sound_move)));
        
        return true;
    }
    
    if(HelperCommandGoTo(Mixer, TA, Assets, Words, WordCount)){
        return true;
    }
    
    TA->Respond(GetVar(Assets, move_invalid_all));
    return false;
}

b8 CommandExit(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    ta_room *Room = TA->CurrentRoom;
    ta_data *Data = TAFindData(&Room->Datas, TADataType_Room, AssetTag(AssetTag_Exit));
    if(!Data){
        // TODO(Tyler): Callback!
        TA->Respond(GetVar(Assets, exit_where));
        return false;
    }
    
    for(u32 I=0; I<Direction_TOTAL; I++){
        if(Room->Adjacents[I] == Data->TAID){
            asset_tag *Tag = &Room->AdjacentTags[I];
            if(HasTag(*Tag, AssetTag_Locked)){
                if(TAAttemptToUnlock(Mixer, TA, Assets, Room, Tag)){
                    TA->Respond(GetVar(Assets, auto_unlock));
                    break;
                }else{
                    TA->Respond(GetVar(Assets, locked));
                    return false;
                }
            }else if(TAIsClosed(TA, *Tag)){
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
b8 CommandEnter(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    // TODO(Tyler): I'm not sure if there is a better way to do this.
    if(WordCount > 1){
        return HelperCommandGoTo(Mixer, TA, Assets, Words, WordCount);
    }
    
    ta_room *Room = TA->CurrentRoom;
    ta_data *Data = TAFindData(&Room->Datas, TADataType_Room, AssetTag(AssetTag_Enter));
    if(!Data){
        TA->Respond(GetVar(Assets, enter_where));
        return false;
    }
    
    for(u32 I=0; I<Direction_TOTAL; I++){
        if(Room->Adjacents[I] == Data->TAID){
            asset_tag *Tag = &Room->AdjacentTags[I];
            if(HasTag(*Tag, AssetTag_Locked)){
                if(TAAttemptToUnlock(Mixer, TA, Assets, Room, Tag)){
                    TA->Respond(GetVar(Assets, auto_unlock));
                    break;
                }else{
                    TA->Respond(GetVar(Assets, locked));
                    return false;
                }
            }else if(TAIsClosed(TA, *Tag)){
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

//~ Item commands

b8 CallbackConfirmBuy(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    ta_room *Room = TA->CurrentRoom;
    ta_item *Item = HashTableFindPtr(&TA->ItemTable, Room->Items[TA->BuyItemIndex]);
    if(!Item){
        LogMessage("Item does not exist!");
        TA->Respond("ERROR!");
        return false;
    }
    
    f32 PositiveWeight = 0.0f;
    f32 NegativeWeight = 0.0f;
    for(u32 I=0; I<WordCount; I++){
        const char *Word = Words[I];
#define WORD(Name) { f32 M = CompareWordsPercentage(Word, Name); if(M > PositiveWeight) PositiveWeight = M; }
        POSITIVES
#undef WORD
#define WORD(Name) { f32 M = CompareWordsPercentage(Word, Name); if(M > NegativeWeight) NegativeWeight = M; }
        NEGATIVES
#undef WORD
    }
    
    if(Maximum(PositiveWeight, NegativeWeight) <= WORD_MATCH_THRESHOLD){
        TA->Respond(GetVar(Assets, buy_callback_prompt), Item->Name);
        TA->Callback = CallbackConfirmBuy;
    }else if(PositiveWeight > NegativeWeight){
        if(TA->AddItem(Room->Items[TA->BuyItemIndex])) TARoomRemoveItem(TA, Room, TA->BuyItemIndex);
        else return false;
        TA->Money -= Item->Cost;
        Item->Dirty = true;
        Item->Cost = 0;
        
        Mixer->PlaySound(GetSoundEffect(Assets, AssetID(sound_item_bought)));
        
        ta_data *Description = TAFindDescription(&Item->Datas, AssetTag(AssetTag_Examine));
        if(Description) TA->Respond(Description->Data);
    }else{
        TA->Respond(GetVar(Assets, buy_callback_no));
    }
    
    return true;
}

b8 CommandTake(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    ta_room *Room = TA->CurrentRoom;
    
    ta_found_items FoundItems = TAFindItems(TA, &Room->Items, Words, WordCount);
    HANDLE_FOUND_ITEMS(FoundItems, CommandTake, "take");
    
    u32 RemovedItems = 0;
    for(u32 I=0; I<FoundItems.Items.Count; I++){
        ta_found_item *FoundItem = &FoundItems.Items[I];
        
        ta_item *Item = FoundItem->Item;
        u32 Index = FoundItem->ItemIndex;
        
        if(HasTag(Item->Tag, AssetTag_Static)){
            ta_data *Description = TAFindDescription(&Item->Datas, AssetTag(AssetTag_Take));
            TA->Respond("%s", Description->Data);
            continue;
        }else if(Item->Cost > 0){
            TA->Respond(GetVar(Assets, take_buy));
            TA->Respond(GetVar(Assets, take_buy_prompt));
            TA->Callback = CallbackConfirmBuy;
            TA->BuyItemIndex = Index-RemovedItems;
            break;
        }
        
        if(TA->AddItem(Room->Items[Index-RemovedItems])) TARoomRemoveItem(TA, Room, Index-RemovedItems);
        else return false;
        RemovedItems++;
        
        ta_data *Description = TAFindDescription(&Item->Datas, AssetTag(AssetTag_Examine));
        if(!Description) return false;
        TA->Respond(Description->Data);
        Mixer->PlaySound(GetSoundEffect(Assets, AssetID(sound_item_taken)));
    }
    
    return true;
}

b8 CommandDrop(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    ta_room *Room = TA->CurrentRoom;
    
    ta_found_items FoundItems = TAFindItems(TA, &TA->Inventory, Words, WordCount);
    HANDLE_FOUND_ITEMS(FoundItems, CommandDrop, "drop");
    
    u32 RemovedItems = 0;
    for(u32 I=0; I<FoundItems.Items.Count; I++){
        ta_found_item *FoundItem = &FoundItems.Items[I];
        
        ta_item *Item = FoundItem->Item;
        u32 Index = FoundItem->ItemIndex;
        
        if(TARoomAddItem(TA, Assets, Room, TA->Inventory[Index-RemovedItems])) ArrayOrderedRemove(&TA->Inventory, Index-RemovedItems);
        RemovedItems++;
        Mixer->PlaySound(GetSoundEffect(Assets, AssetID(sound_item_dropped)));
    }
    
    return true;
}

b8 CommandBuy(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    ta_room *Room = TA->CurrentRoom;
    
    ta_found_items FoundItems = TAFindItems(TA, &Room->Items, Words, WordCount);
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
            if(TA->AddItem(Room->Items[Index-RemovedItems])) TARoomRemoveItem(TA, Room, Index-RemovedItems);
            else return false;
            RemovedItems++;
            
            ta_data *Description = TAFindDescription(&Item->Datas, AssetTag(AssetTag_Examine));
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

b8 CommandEat(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    ta_found_items FoundItems = TAFindItems(TA, &TA->Inventory, Words, WordCount);
    HANDLE_FOUND_ITEMS(FoundItems, CommandEat, "eat");
    
    u32 RemovedItems = 0;
    for(u32 I=0; I<FoundItems.Items.Count; I++){
        ta_found_item *FoundItem = &FoundItems.Items[I];
        
        ta_item *Item = FoundItem->Item;
        u32 Index = FoundItem->ItemIndex;
        
        ta_data *Description = TAFindDescription(&Item->Datas, AssetTag(AssetTag_Eat));
        if(!Description){
            TA->Respond(GetVar(Assets, eat_cant));
            continue;
        }
        TA->Respond(Description->Data);
        
        if(HasTag(Item->Tag, AssetTag_Bread)){
            TA->AddItem(TAItemByName(TA, "bread crumbs"));
        }
        
        ArrayOrderedRemove(&TA->Inventory, Index-RemovedItems);
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

b8 CommandPlay(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    ta_room *Room = TA->CurrentRoom;
    
    ta_found_items FoundItems = TAFindItems(TA, &TA->Inventory, Words, WordCount);
    TAContinueFindItems(TA, &Room->Items, Words, WordCount, &FoundItems);
    HANDLE_FOUND_ITEMS(FoundItems, CommandPlay, "play");
    
    for(u32 I=0; I<FoundItems.Items.Count; I++){
        ta_found_item *FoundItem = &FoundItems.Items[I];
        
        ta_item *Item = FoundItem->Item;
        
        asset_tag Tag = AssetTag(AssetTag_Play);
        if(HasTag(Item->Tag, AssetTag_Organ)){
            ta_data *Sound = TAFindData(&Item->Datas, TADataType_Asset, AssetTag(AssetTag_Sound, AssetTag_Play, TA->OrganState));
            if(Sound){
                Mixer->PlaySound(GetSoundEffect(Assets, Sound->Asset));
            }
            Tag.B = (u8)TA->OrganState;
        }
        
        ta_data *Description = TAFindDescription(&Item->Datas, Tag);
        if(!Description) continue;;
        TA->Respond(Description->Data);
    }
    
    return true;
}

b8 CommandExamine(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    ta_room *Room = TA->CurrentRoom;
    
    ta_found_items FoundItems = TAFindItems(TA, &TA->Inventory, Words, WordCount);
    TAContinueFindItems(TA, &Room->Items, Words, WordCount, &FoundItems);
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
        
        ta_data *Description = TAFindDescription(&Item->Datas, Tag);
        if(!Description) continue;
        TA->Respond(Description->Data);
    }
    
    return true;
}

b8 CommandUnlock(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    ta_room *CurrentRoom = TA->CurrentRoom;
    for(u32 I=0; I<Direction_TOTAL; I++){
        asset_tag Tag = CurrentRoom->AdjacentTags[I];
        if(!HasTag(Tag, AssetTag_Locked)) continue;
        if(TAAttemptToUnlock(Mixer, TA, Assets, CurrentRoom, &Tag)){
            TA->Respond(GetVar(Assets, auto_unlock));
        }else{
            TA->Respond(GetVar(Assets, locked));
        }
    }
    
    return true;
}

b8 CommandWait(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    TA->Respond("Not yet implemented!");
    return true;
}

b8 CommandRepair(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    ta_room *Room = TA->CurrentRoom;
    
    ta_found_items FoundItems =  TAFindItems(TA, &TA->Inventory, Words, WordCount);
    TAContinueFindItems(TA, &Room->Items, Words, WordCount, &FoundItems);
    
    ta_item *Item = 0;
    b8 FixedSomething = false;
    for(u32 I=0; I<FoundItems.Items.Count; I++){
        ta_found_item *FoundItem = &FoundItems.Items[I];
        Item = FoundItem->Item;
        
        if(!HasTag(Item->Tag, AssetTag_Broken)){
            continue;
        }
        
        ta_data *Data = TAFindData(&Item->Datas, TADataType_Item, AssetTag(AssetTag_Fixer));
        if(!Data){
            TA->Respond(GetVar(Assets, repair_cant));
            continue;
        }
        
        for(u32 J=0; J<TA->Inventory.Count; J++){
            ta_id FixerItemID = TA->Inventory[J];
            if(FixerItemID != Data->TAID) continue;
            
            SwitchTag(&Item->Tag, AssetTag_Broken, AssetTag_Repaired);
            ArrayOrderedRemove(&TA->Inventory, J);
            ta_data *Sound = TAFindData(&Item->Datas, TADataType_Asset, AssetTag(AssetTag_Sound, AssetTag_Repaired));
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

b8 CommandUse(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    ta_room *Room = TA->CurrentRoom;
    
    ta_found_items FoundItems =  TAFindItems(TA, &TA->Inventory, Words, WordCount);
    //TAContinueFindItems(TA, &Room->Items, Words, WordCount, &FoundItems);
    
    for(u32 I=0; I<FoundItems.Items.Count; I++){
        ta_found_item *FoundItem = &FoundItems.Items[I];
        ta_item *Item = FoundItem->Item;
        
        ta_data *Data = TAFindData(&Item->Datas, TADataType_Command, AssetTag(AssetTag_Use));
        if(!Data){
            TA->Respond(GetVar(Assets, use_dont_know));
            continue;
        }
        
        u32 TokenCount;
        char **Tokens = TokenizeCommand(&TransientStorageArena, Data->Data, &TokenCount);
        TADispatchCommand(Mixer, TA, Assets, Tokens, TokenCount);
        
        return true;
    }
    
    TA->Respond(GetVar(Assets, use_dont_have));
    return false;
}

//~ Miscellaneous commands
b8 CommandPray(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    ta_room *Room = TA->CurrentRoom;
    
    if(HasTag(Room->Tag, AssetTag_Altar)){
        //if();
        TA->Respond(GetVar(Assets, pray_first));
    }
    
    return true;
}

//~ Testing commands
b8 CommandTestAddMoney(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    TA->Money += 10;
    return true;
}

b8 CommandTestSubMoney(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    if(TA->Money >= 10) TA->Money -= 10;
    return true;
}
