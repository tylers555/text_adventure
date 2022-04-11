//~ Commands
void CommandMove(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    ta_room *CurrentRoom = TA->CurrentRoom;
    ta_room *NextRoom = 0;
    
    for(u32 I=0; I < WordCount; I++){
        char *Word = Words[I];
        direction Direction = HashTableFind(&TA->DirectionTable, (const char *)Word);
        if(!Direction) continue;
        
        ta_id NextRoomString = CurrentRoom->Adjacents[Direction];
        NextRoom = HashTableFindPtr(&TA->RoomTable, NextRoomString);
        if(!NextRoom){
            TA->Respond("Why would you want move that way!?\nDoing so would be quite \002\002foolish\002\001!");
            return;
        }
        
        asset_tag *Tag = &CurrentRoom->AdjacentTags[Direction];
        if(HasTag(*Tag, AssetTag_Locked)){
            if(TAAttemptToUnlock(Mixer, TA, Assets, CurrentRoom, Tag)){
                TA->Respond("(unlocked)");
                break;
            }else{
                TA->Respond("That way is locked, \002\002buddy-o\002\001!");
                return;
            }
        }else if(TAIsClosed(TA, *Tag)){
            return;
        }
        
        break;
    }
    
    if(!NextRoom){
        TA->Respond("Don't you understand how to move!?\nYou need to specify a direction, \002\002pal\002\001!");
        return;
    }
    TA->CurrentRoom = NextRoom;
    
    Mixer->PlaySound(GetSoundEffect(Assets, AssetID(sound_move)));
}

void CommandTake(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
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
            TA->Respond("You're going to have to \002\002buy\002\001 that!");
            continue;
        }
        
        if(TA->AddItem(Room->Items[Index-RemovedItems])) TARoomRemoveItem(TA, Room, Index-RemovedItems);
        else return;
        RemovedItems++;
        
        ta_data *Description = TAFindDescription(&Item->Datas, AssetTag(AssetTag_Examine));
        if(!Description) return;
        TA->Respond(Description->Data);
        Mixer->PlaySound(GetSoundEffect(Assets, AssetID(sound_item_taken)));
    }
}

void CommandDrop(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
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
}

void CallbackConfirmBuy(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    Assert(0);
}

void CommandBuy(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    ta_room *Room = TA->CurrentRoom;
    
    ta_found_items FoundItems = TAFindItems(TA, &Room->Items, Words, WordCount);
    HANDLE_FOUND_ITEMS(FoundItems, CommandBuy, "buy");
    
    u32 RemovedItems = 0;
    for(u32 I=0; I<FoundItems.Items.Count; I++){
        ta_found_item *FoundItem = &FoundItems.Items[I];
        
        ta_item *Item = FoundItem->Item;
        u32 Index = FoundItem->ItemIndex;
        
        if(HasTag(Item->Tag, AssetTag_Static)){
            TA->Respond("You couldn't possible hope to buy that!");
            continue;
        }
        
        if(Item->Cost == 0){
            TA->Respond("You don't have to \002\002pay\002\001 for that!");
            if(TA->AddItem(Room->Items[Index-RemovedItems])) TARoomRemoveItem(TA, Room, Index-RemovedItems);
            else return;
            RemovedItems++;
            
            ta_data *Description = TAFindDescription(&Item->Datas, AssetTag(AssetTag_Examine));
            if(!Description) continue;
            TA->Respond(Description->Data);
            Mixer->PlaySound(GetSoundEffect(Assets, AssetID(sound_item_taken)));
        }else if(TA->Money >= Item->Cost){
            if(TA->AddItem(Room->Items[Index-RemovedItems])) TARoomRemoveItem(TA, Room, Index-RemovedItems);
            else return;
            RemovedItems++;
            TA->Money -= Item->Cost;
            Item->Dirty = true;
            Item->Cost = 0;
            
            ta_data *Description = TAFindDescription(&Item->Datas, AssetTag(AssetTag_Examine));
            if(!Description) continue;
            TA->Respond(Description->Data);
            Mixer->PlaySound(GetSoundEffect(Assets, AssetID(sound_item_bought)));
        }else{
            TA->Respond("You don't have enough \002\002money\002\001 for that!");
            continue;
        }
    }
}

void CommandEat(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    ta_found_items FoundItems = TAFindItems(TA, &TA->Inventory, Words, WordCount);
    HANDLE_FOUND_ITEMS(FoundItems, CommandEat, "eat");
    
    u32 RemovedItems = 0;
    for(u32 I=0; I<FoundItems.Items.Count; I++){
        ta_found_item *FoundItem = &FoundItems.Items[I];
        
        ta_item *Item = FoundItem->Item;
        u32 Index = FoundItem->ItemIndex;
        
        ta_data *Description = TAFindDescription(&Item->Datas, AssetTag(AssetTag_Eat));
        if(!Description){
            TA->Respond("You can't eat \002\002that\002\001!");
            continue;
        }
        TA->Respond(Description->Data);
        
        if(HasTag(Item->Tag, AssetTag_Bread)){
            TA->AddItem(TAItemByName(TA, "bread crumbs"));
        }
        
        ArrayOrderedRemove(&TA->Inventory, Index-RemovedItems);
        RemovedItems++;
    }
    
    Mixer->PlaySound(GetSoundEffect(Assets, AssetID(sound_item_eaten)));
}

void CommandPlay(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
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
}

void CommandExamine(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
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
}

void CommandUnlock(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    ta_room *CurrentRoom = TA->CurrentRoom;
    for(u32 I=0; I<Direction_TOTAL; I++){
        asset_tag Tag = CurrentRoom->AdjacentTags[I];
        if(!HasTag(Tag, AssetTag_Locked)) continue;
        if(TAAttemptToUnlock(Mixer, TA, Assets, CurrentRoom, &Tag)){
            TA->Respond("Unlocked!");
        }else{
            TA->Respond("You can't unlock that!");
        }
    }
}

void CommandWait(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    
}

void CommandRepair(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
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
        
        ta_data *Data = TAFindData(&Item->Datas, TADataType_Description, AssetTag(AssetTag_Fixer));
        if(!Data){
            TA->Respond("That cannot be fixed!");
            continue;
        }
        
        for(u32 J=0; J<TA->Inventory.Count; J++){
            ta_id FixerItemID = TA->Inventory[J];
            ta_item *FixerItem = HashTableFindPtr(&TA->ItemTable, FixerItemID);
            if(!FixerItem) return;
            if(!HasTag(FixerItem->Tag, AssetTag_Fixer)) continue;
            if(!CompareStrings(Data->Data, FixerItem->Name)){
                continue;
            }
            
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
    
    if(!FixedSomething && Item && !HasTag(Item->Tag, AssetTag_Broken)) TA->Respond("That is not broken!");
}

//~ Testing commands
void CommandTestAddMoney(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    TA->Money += 10;
}

void CommandTestSubMoney(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    if(TA->Money >= 10) TA->Money -= 10;
}
