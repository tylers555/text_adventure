//~ Commands
void CommandMove(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    ta_room *CurrentRoom = TA->CurrentRoom;
    ta_room *NextRoom = 0;
    
    for(u32 I=0; I < WordCount; I++){
        char *Word = Words[I];
        direction Direction = FindInHashTable(&TA->DirectionTable, (const char *)Word);
        if(!Direction) continue;
        
        string NextRoomString = CurrentRoom->Adjacents[Direction];
        NextRoom = FindInHashTablePtr(&TA->RoomTable, NextRoomString);
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
    
    Mixer->PlaySound(Assets->GetSoundEffect(String("room_change")));
}

void CommandTake(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    ta_room *Room = TA->CurrentRoom;
    
    ta_found_items FoundItems = TAFindItems(TA, &Room->Items, Words, WordCount);
    if(FoundItems.IsAmbiguous){
        TA->Respond("You will have to be more specific, what item do you want to take?\nEnter the item: ");
        TA->Callback = CommandTake;
        return;
    }
    
    if(!FoundItems.Items.Count){
        TA->Respond("I have no idea what you want to take!");
        return;
    }
    
    u32 RemovedItems = 0;
    for(u32 I=0; I<FoundItems.Items.Count; I++){
        ta_found_item *FoundItem = &FoundItems.Items[I];
        
        ta_item *Item = FoundItem->Item;
        u32 Index = FoundItem->ItemIndex;
        
        if(HasTag(Item->Tag, AssetTag_Static)){
            ta_string *Description = TAFindDescription(&Item->Descriptions, AssetTag(AssetTag_Take));
            TA->Respond("%s", Description->Data);
            continue;
        }else if(Item->Cost > 0){
            TA->Respond("You're going to have to \002\002buy\002\001 that!");
            continue;
        }
        
        if(TA->AddItem(Room->Items[Index-RemovedItems])) TARoomRemoveItem(TA, Room, Index-RemovedItems);
        else return;
        RemovedItems++;
        
        ta_string *Description = TAFindDescription(&Item->Descriptions, AssetTag(AssetTag_Examine));
        if(!Description) return;
        TA->Respond(Description->Data);
        Mixer->PlaySound(Assets->GetSoundEffect(String("item_taken")));
    }
}

void CommandDrop(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    ta_room *Room = TA->CurrentRoom;
    
    ta_found_items FoundItems = TAFindItems(TA, &TA->Inventory, Words, WordCount);
    if(FoundItems.IsAmbiguous){
        TA->Respond("You will have to be more specific, what item do you want to drop?\nEnter the item: ");
        TA->Callback = CommandDrop;
        return;
    }
    
    if(!FoundItems.Items.Count){
        TA->Respond("I have no idea what you want to drop!");
        return;
    }
    
    u32 RemovedItems = 0;
    for(u32 I=0; I<FoundItems.Items.Count; I++){
        ta_found_item *FoundItem = &FoundItems.Items[I];
        
        ta_item *Item = FoundItem->Item;
        u32 Index = FoundItem->ItemIndex;
        
        if(TARoomAddItem(TA, Assets, Room, TA->Inventory[Index-RemovedItems])) ArrayOrderedRemove(&TA->Inventory, Index-RemovedItems);
        RemovedItems++;
        Mixer->PlaySound(Assets->GetSoundEffect(String("item_dropped")));
    }
}

void CallbackConfirmBuy(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    Assert(0);
}

void CommandBuy(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    ta_room *Room = TA->CurrentRoom;
    
    ta_found_items FoundItems = TAFindItems(TA, &Room->Items, Words, WordCount);
    if(FoundItems.IsAmbiguous){
        TA->Respond("You will have to be more specific, what item do you want to buy?\nEnter the item: ");
        TA->Callback = CommandBuy;
        return;
    }
    
    if(!FoundItems.Items.Count){
        TA->Respond("I have no idea what you want to buy!");
        return;
    }
    
    u32 RemovedItems = 0;
    for(u32 I=0; I<FoundItems.Items.Count; I++){
        ta_found_item *FoundItem = &FoundItems.Items[I];
        
        ta_item *Item = FoundItem->Item;
        u32 Index = FoundItem->ItemIndex;
        
        if(HasTag(Item->Tag, AssetTag_Static)){
            TA->Respond("You couldn't possible hope to buy that!");
            continue;
        }
        if(TA->Money >= Item->Cost){
            if(TA->AddItem(Room->Items[Index-RemovedItems])) TARoomRemoveItem(TA, Room, Index-RemovedItems);
            else return;
            RemovedItems++;
            TA->Money -= Item->Cost;
            Item->Dirty = true;
            Item->Cost = 0;
        }else{
            TA->Respond("You don't have enough \002\002money\002\001 for that!");
            continue;
        }
        
        ta_string *Description = TAFindDescription(&Item->Descriptions, AssetTag(AssetTag_Examine));
        if(!Description) continue;
        TA->Respond(Description->Data);
        Mixer->PlaySound(Assets->GetSoundEffect(String("item_bought")));
    }
}

void CommandEat(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    ta_found_items FoundItems = TAFindItems(TA, &TA->Inventory, Words, WordCount);
    
    if(FoundItems.IsAmbiguous){
        TA->Respond("You will have to be more specific, what item do you want to eat?\nEnter the item: ");
        TA->Callback = CommandEat;
        return;
    }
    
    if(!FoundItems.Items.Count){
        TA->Respond("I have no idea what you want to eat!");
        return;
    }
    
    u32 RemovedItems = 0;
    for(u32 I=0; I<FoundItems.Items.Count; I++){
        ta_found_item *FoundItem = &FoundItems.Items[I];
        
        ta_item *Item = FoundItem->Item;
        u32 Index = FoundItem->ItemIndex;
        
        ta_string *Description = TAFindDescription(&Item->Descriptions, AssetTag(AssetTag_Eat));
        if(!Description){
            TA->Respond("You can't eat \002\002that\002\001!");
            continue;
        }
        TA->Respond(Description->Data);
        
        if(HasTag(Item->Tag, AssetTag_Bread)){
            TA->AddItem(String("bread crumbs"));
        }
        
        ArrayOrderedRemove(&TA->Inventory, Index-RemovedItems);
        RemovedItems++;
    }
    
    Mixer->PlaySound(Assets->GetSoundEffect(String("item_eaten")));
}

void CommandPlay(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    ta_room *Room = TA->CurrentRoom;
    
    ta_found_items FoundItems = TAFindItems(TA, &TA->Inventory, Words, WordCount);
    TAContinueFindItems(TA, &Room->Items, Words, WordCount, &FoundItems);
    
    if(FoundItems.IsAmbiguous){
        TA->Respond("You will have to be more specific, what item do you want to play?\nEnter the item: ");
        TA->Callback = CommandPlay;
        return;
    }
    
    if(!FoundItems.Items.Count){
        TA->Respond("I have no idea what you want to play!");
        return;
    }
    
    for(u32 I=0; I<FoundItems.Items.Count; I++){
        ta_found_item *FoundItem = &FoundItems.Items[I];
        
        ta_item *Item = FoundItem->Item;
        
        asset_tag Tag = AssetTag(AssetTag_Play);
        if(HasTag(Item->Tag, AssetTag_Organ)){
            string Sound;
            if(TA->OrganState == AssetTag_Broken) Sound = String("organ_play_broken");
            else                                  Sound = String("organ_play_repaired");
            Mixer->PlaySound(Assets->GetSoundEffect(Sound));
            Tag = AssetTag(AssetTag_Play, TA->OrganState);
        }
        
        ta_string *Description = TAFindDescription(&Item->Descriptions, Tag);
        if(!Description) continue;;
        TA->Respond(Description->Data);
    }
}

void CommandExamine(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    ta_room *Room = TA->CurrentRoom;
    
    ta_found_items FoundItems = TAFindItems(TA, &TA->Inventory, Words, WordCount);
    TAContinueFindItems(TA, &Room->Items, Words, WordCount, &FoundItems);
    
    if(FoundItems.IsAmbiguous){
        TA->Respond("You will have to be more specific, what item do you want to examine?\nEnter the item: ");
        TA->Callback = CommandExamine;
        return;
    }
    
    if(!FoundItems.Items.Count){
        TA->Respond("I have no idea what you want to examine!");
        return;
    }
    
    for(u32 I=0; I<FoundItems.Items.Count; I++){
        ta_found_item *FoundItem = &FoundItems.Items[I];
        
        ta_item *Item = FoundItem->Item;
        
        asset_tag Tag = AssetTag(AssetTag_Examine);
        if(HasTag(Item->Tag, AssetTag_Organ)){
            Tag = AssetTag(AssetTag_Examine, TA->OrganState);
        }
        
        ta_string *Description = TAFindDescription(&Item->Descriptions, Tag);
        if(!Description) continue;
        TA->Respond(Description->Data);
    }
}

void CommandMap(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    TA->Respond("Not implemented yet!");
    Assert(0);
}

void CommandUnlock(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    ta_room *CurrentRoom = TA->CurrentRoom;
    for(u32 I=0; I<Direction_TOTAL; I++){
        asset_tag Tag = CurrentRoom->AdjacentTags[I];
        if(!HasTag(Tag, AssetTag_Locked)) continue;
        if(TAAttemptToUnlock(Mixer, TA, Assets, CurrentRoom, &Tag)){
            TA->Respond("Unlocked!");
        }else{
            TA->Respond("you can't unlock that!");
        }
    }
}

//~ Testing commands
void CommandTestRepair(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    if(TA->OrganState == AssetTag_Broken)        TA->OrganState = AssetTag_Repaired;
    else if(TA->OrganState == AssetTag_Repaired) TA->OrganState = AssetTag_Broken;
}

void CommandTestAddMoney(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    TA->Money += 10;
}

void CommandTestSubMoney(audio_mixer *Mixer, ta_system *TA, asset_system *Assets, char **Words, u32 WordCount){
    if(TA->Money >= 10) TA->Money -= 10;
}
