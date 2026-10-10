#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Struct/PlayerDatas.h"
#include "ES_GameInstance.generated.h"

class UES_SaveGame;
class UDataTable;

// holds static DataTables (hard reference : always cooked, same on server and client)
// lookup goes through UDataManagerSubsystem
UCLASS()
class EXTRACTIONSHOOTER14_API UES_GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UDataTable* GetShopItemTable() const { return ShopItemTable; }
	UDataTable* GetWeaponTable() const { return WeaponTable; }
	UDataTable* GetUseItemTable() const { return UseItemTable; }

protected:
	// every row is a shop item (row = purchasable)
	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TObjectPtr<UDataTable> ShopItemTable;

	// weapon stats + weapon mesh
	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TObjectPtr<UDataTable> WeaponTable;

	// use item (bandage, medkit ...) effect + world mesh
	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TObjectPtr<UDataTable> UseItemTable;
};
