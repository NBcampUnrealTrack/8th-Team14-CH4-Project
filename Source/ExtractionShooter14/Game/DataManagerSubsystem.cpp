#include "DataManagerSubsystem.h"

#include "ES_GameInstance.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "GameMapsSettings.h"

namespace
{
	// tables of the configured GameInstance class (editor construction has no GameInstance)
	const UES_GameInstance* GetGameInstanceDefaults()
	{
		const UGameMapsSettings* MapsSettings = GetDefault<UGameMapsSettings>();
		UClass* GameInstanceClass = MapsSettings ? MapsSettings->GameInstanceClass.TryLoadClass<UES_GameInstance>() : nullptr;
		return GameInstanceClass ? GameInstanceClass->GetDefaultObject<UES_GameInstance>() : nullptr;
	}

	template <typename RowType>
	const RowType* FindRowIn(const UDataTable* Table, FName RowName, const TCHAR* Context)
	{
		if (!IsValid(Table) || RowName.IsNone()) return nullptr;
		return Table->FindRow<RowType>(RowName, Context, false);
	}

	// ItemType decides which performance table DataRowName points into
	const FWeaponData* FindWeaponIn(const UDataTable* Weapons, const FItemData& Item)
	{
		return Item.ItemType == EItemType::Weapon ? FindRowIn<FWeaponData>(Weapons, Item.DataRowName, TEXT("FindWeapon")) : nullptr;
	}

	const FUseItemData* FindUseItemIn(const UDataTable* UseItems, const FItemData& Item)
	{
		return Item.ItemType == EItemType::UseItem ? FindRowIn<FUseItemData>(UseItems, Item.DataRowName, TEXT("FindUseItem")) : nullptr;
	}
}

void UDataManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UES_GameInstance* GameInstanceES = Cast<UES_GameInstance>(GetGameInstance());
	if (!IsValid(GameInstanceES))
	{
		UE_LOG(LogTemp, Error, TEXT("[DataManager] GameInstance is not UES_GameInstance. Set Project Settings > Maps & Modes > Game Instance Class."));
		return;
	}

	ShopItemTable = GameInstanceES->GetShopItemTable();
	WeaponTable = GameInstanceES->GetWeaponTable();
	UseItemTable = GameInstanceES->GetUseItemTable();

	if (!IsValid(ShopItemTable)) UE_LOG(LogTemp, Error, TEXT("[DataManager] ShopItemTable is not set in GameInstance."));
	if (!IsValid(WeaponTable)) UE_LOG(LogTemp, Error, TEXT("[DataManager] WeaponTable is not set in GameInstance."));
	if (!IsValid(UseItemTable)) UE_LOG(LogTemp, Error, TEXT("[DataManager] UseItemTable is not set in GameInstance."));
}

#pragma region C++
const FItemData* UDataManagerSubsystem::FindShopItem(FName ItemName) const
{
	return FindRowIn<FItemData>(ShopItemTable, ItemName, TEXT("FindShopItem"));
}

const FWeaponData* UDataManagerSubsystem::FindWeapon(FName WeaponRowName) const
{
	return FindRowIn<FWeaponData>(WeaponTable, WeaponRowName, TEXT("FindWeapon"));
}

const FUseItemData* UDataManagerSubsystem::FindUseItem(FName UseItemRowName) const
{
	return FindRowIn<FUseItemData>(UseItemTable, UseItemRowName, TEXT("FindUseItem"));
}

const FWeaponData* UDataManagerSubsystem::FindWeaponFor(const FItemData& Item) const
{
	return FindWeaponIn(WeaponTable, Item);
}

const FUseItemData* UDataManagerSubsystem::FindUseItemFor(const FItemData& Item) const
{
	return FindUseItemIn(UseItemTable, Item);
}

const FWeaponData* UDataManagerSubsystem::FindWeaponForShopItem(FName ItemName) const
{
	const FItemData* Item = FindShopItem(ItemName);
	return Item ? FindWeaponFor(*Item) : nullptr;
}

const FUseItemData* UDataManagerSubsystem::FindUseItemForShopItem(FName ItemName) const
{
	const FItemData* Item = FindShopItem(ItemName);
	return Item ? FindUseItemFor(*Item) : nullptr;
}

TOptional<int32> UDataManagerSubsystem::FindPrice(FName ItemName) const
{
	const FItemData* Item = FindShopItem(ItemName);
	if (!Item) return {};

	if (const FWeaponData* Weapon = FindWeaponFor(*Item)) return Weapon->Price;
	if (const FUseItemData* UseItem = FindUseItemFor(*Item)) return UseItem->Price;
	return {};
}

UStaticMesh* UDataManagerSubsystem::FindWorldMesh(const UObject* WorldContext, const FItemData& Item)
{
	const UGameInstance* GameInstance = WorldContext ? WorldContext->GetWorld() ? WorldContext->GetWorld()->GetGameInstance() : nullptr : nullptr;
	const UDataManagerSubsystem* DataManager = GameInstance ? GameInstance->GetSubsystem<UDataManagerSubsystem>() : nullptr;

	// runtime : subsystem tables / editor : GameInstance class defaults
	const UES_GameInstance* Defaults = DataManager ? nullptr : GetGameInstanceDefaults();
	const UDataTable* Weapons = DataManager ? DataManager->WeaponTable.Get() : (Defaults ? Defaults->GetWeaponTable() : nullptr);
	const UDataTable* UseItems = DataManager ? DataManager->UseItemTable.Get() : (Defaults ? Defaults->GetUseItemTable() : nullptr);

	if (const FWeaponData* Weapon = FindWeaponIn(Weapons, Item))
	{
		return Weapon->WeaponMesh;
	}
	if (const FUseItemData* UseItem = FindUseItemIn(UseItems, Item))
	{
		return UseItem->Mesh;
	}
	return nullptr;
}
#pragma endregion

#pragma region Blueprint
bool UDataManagerSubsystem::GetShopItem(FName ItemName, FItemData& OutItem) const
{
	const FItemData* Item = FindShopItem(ItemName);
	if (!Item) return false;

	OutItem = *Item;
	return true;
}

bool UDataManagerSubsystem::GetWeaponForShopItem(FName ItemName, FWeaponData& OutWeapon) const
{
	const FWeaponData* Weapon = FindWeaponForShopItem(ItemName);
	if (!Weapon) return false;

	OutWeapon = *Weapon;
	return true;
}

bool UDataManagerSubsystem::GetUseItemForShopItem(FName ItemName, FUseItemData& OutUseItem) const
{
	const FUseItemData* UseItem = FindUseItemForShopItem(ItemName);
	if (!UseItem) return false;

	OutUseItem = *UseItem;
	return true;
}

bool UDataManagerSubsystem::GetShopItemDisplay(FName ItemName, FShopItemDisplay& OutDisplay) const
{
	OutDisplay = FShopItemDisplay();

	const FItemData* Item = FindShopItem(ItemName);
	if (!Item) return false;

	// how it looks : DT_ShopItem
	OutDisplay.ItemName = ItemName;
	OutDisplay.DisplayName = Item->DisplayName;
	OutDisplay.ItemImage = Item->Icon;
	OutDisplay.PrimaryIcon = Item->UIIcon_1;
	OutDisplay.SecondaryIcon = Item->UIIcon_2;
	OutDisplay.PurchaseSound = Item->PurchaseSound;

	// weapon : DT_Weapon
	if (const FWeaponData* Weapon = FindWeaponFor(*Item))
	{
		OutDisplay.bIsWeapon = true;
		OutDisplay.Price = Weapon->Price;
		OutDisplay.PrimaryValue = Weapon->Damage;
		OutDisplay.SecondaryValue = Weapon->MagazineSize;
		return true;
	}

	// use item : DT_UseItem
	if (const FUseItemData* UseItem = FindUseItemFor(*Item))
	{
		OutDisplay.Price = UseItem->Price;
		OutDisplay.PrimaryValue = UseItem->EffectAmount;
		OutDisplay.SecondaryValue = UseItem->UseTime;
		return true;
	}

	// no performance row for its ItemType : not a shop item
	return false;
}
#pragma endregion
