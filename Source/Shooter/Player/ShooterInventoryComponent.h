#pragma once

#include "CoreMinimal.h"
#include "UObject/UObjectGlobals.h"
#include "Components/ActorComponent.h"
#include "Items/ShooterAmmoType.h"
#include "ShooterInventoryComponent.generated.h"

/** forward declarations */
class AShooterItem;

UCLASS()
class UShooterInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UShooterInventoryComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	/** Map to keep track of ammo of the different ammo types */
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items, meta = (AllowPrivateAccess = "true"))
	//TMap<EShooterAmmoType, int32> AmmoMap;

	/** An Array of AShooterItem for our Inventory */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	TArray<AShooterItem*> Slots;

	const int32 CAPACITY{ 6 };

public:
	bool AddItem(AShooterItem* InItem);
	bool SetItem(int32 SlotIndex, AShooterItem* InItem);
	int32 GetEmptySlot();
	AShooterItem* GetItem(int SlotIndex) const { return Slots[SlotIndex]; }
	FORCEINLINE int32 Capacity() const { return CAPACITY; }
	FORCEINLINE int32 Size() const { return Slots.Num(); }
	FORCEINLINE bool HasEmptySlot() const { return Size() < Capacity(); }
};
