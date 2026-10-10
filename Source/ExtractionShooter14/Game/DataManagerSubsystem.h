#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Item/ItemData.h"
#include "Item/UseItemData.h"
#include "Weapon/WeaponData.h"
#include "DataManagerSubsystem.generated.h"

class UDataTable;
class UTexture2D;
class USoundBase;
class UStaticMesh;

// everything one shop row shows, already combined from the tables
USTRUCT(BlueprintType)
struct FShopItemDisplay
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName ItemName;
	UPROPERTY(BlueprintReadOnly)
	FText DisplayName;
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UTexture2D> ItemImage = nullptr;
	// weapon : DT_Weapon / use item : DT_UseItem
	UPROPERTY(BlueprintReadOnly)
	int32 Price = 0;

	// weapon : Damage / use item : EffectAmount
	UPROPERTY(BlueprintReadOnly)
	float PrimaryValue = 0.f;
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UTexture2D> PrimaryIcon = nullptr;

	// weapon : MagazineSize / use item : UseTime
	UPROPERTY(BlueprintReadOnly)
	float SecondaryValue = 0.f;
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UTexture2D> SecondaryIcon = nullptr;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<USoundBase> PurchaseSound = nullptr;
	UPROPERTY(BlueprintReadOnly)
	bool bIsWeapon = false;
};

// single entry point for static DataTables (server and client both have one)
// tables are owned by UES_GameInstance, this subsystem only looks them up
//   DT_ShopItem : display (name, icons, sound) + ItemType + DataRowName
//   DT_Weapon   : weapon price + performance
//   DT_UseItem  : use item price + performance
UCLASS()
class EXTRACTIONSHOOTER14_API UDataManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

#pragma region C++
	const FItemData* FindShopItem(FName ItemName) const;
	const FWeaponData* FindWeapon(FName WeaponRowName) const;
	const FUseItemData* FindUseItem(FName UseItemRowName) const;

	// ItemType picks the table, DataRowName finds the row (nullptr when not that kind)
	const FWeaponData* FindWeaponFor(const FItemData& Item) const;
	const FUseItemData* FindUseItemFor(const FItemData& Item) const;

	// shop item -> linked performance row
	const FWeaponData* FindWeaponForShopItem(FName ItemName) const;
	const FUseItemData* FindUseItemForShopItem(FName ItemName) const;

	// price from the linked table (unset when the shop item links nothing = not purchasable)
	TOptional<int32> FindPrice(FName ItemName) const;
	bool IsPurchasable(FName ItemName) const { return FindPrice(ItemName).IsSet(); }

	// world look : Weapon -> WeaponMesh, UseItem -> UseItem Mesh
	// works without a GameInstance (editor construction) by reading GameInstance class defaults
	static UStaticMesh* FindWorldMesh(const UObject* WorldContext, const FItemData& Item);
#pragma endregion

#pragma region Blueprint
	UFUNCTION(BlueprintPure, Category = "Data")
	UDataTable* GetShopItemTable() const { return ShopItemTable; }

	UFUNCTION(BlueprintPure, Category = "Data")
	UDataTable* GetWeaponTable() const { return WeaponTable; }

	UFUNCTION(BlueprintPure, Category = "Data")
	UDataTable* GetUseItemTable() const { return UseItemTable; }

	UFUNCTION(BlueprintCallable, Category = "Data")
	bool GetShopItem(FName ItemName, FItemData& OutItem) const;

	UFUNCTION(BlueprintCallable, Category = "Data")
	bool GetWeaponForShopItem(FName ItemName, FWeaponData& OutWeapon) const;

	UFUNCTION(BlueprintCallable, Category = "Data")
	bool GetUseItemForShopItem(FName ItemName, FUseItemData& OutUseItem) const;

	// the only place that combines the tables for shop display
	UFUNCTION(BlueprintCallable, Category = "Data|Shop")
	bool GetShopItemDisplay(FName ItemName, FShopItemDisplay& OutDisplay) const;
#pragma endregion

private:
	UPROPERTY()
	TObjectPtr<UDataTable> ShopItemTable;

	UPROPERTY()
	TObjectPtr<UDataTable> WeaponTable;

	UPROPERTY()
	TObjectPtr<UDataTable> UseItemTable;
};
