#include "Item/HealItem.h"

#include "Game/DataManagerSubsystem.h"
#include "Engine/GameInstance.h"

float AHealItem::GetHealAmount() const
{
    // heal amount lives in DT_UseItem
    const UGameInstance* GameInstance = GetGameInstance();
    const UDataManagerSubsystem* DataManager = GameInstance ? GameInstance->GetSubsystem<UDataManagerSubsystem>() : nullptr;
    const FUseItemData* UseItem = DataManager ? DataManager->FindUseItemFor(CurrentItemData) : nullptr;

    return UseItem ? UseItem->EffectAmount : 0.f;
}