#include "ShooterInventoryComponent.h"
#include UE_INLINE_GENERATED_CPP_BY_NAME(ShooterInventoryComponent)

UShooterInventoryComponent::UShooterInventoryComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

bool UShooterInventoryComponent::AddItem(AShooterItem* InItem)
{
	if (Slots.Num() < CAPACITY)
	{
		Slots.Add(InItem);
		return true;
	}
	return false;	
}

bool UShooterInventoryComponent::SetItem(int32 SlotIndex, AShooterItem* InItem)
{
	if (!Slots.IsValidIndex(SlotIndex)) return false;

	Slots[SlotIndex] = InItem;

	return true;
}

int32 UShooterInventoryComponent::GetEmptySlot()
{
	for (int32 i = 0; i < Slots.Num(); i++)
	{
		if (Slots[i] == nullptr)
		{
			return i;
		}
	}
	if (Slots.Num() < CAPACITY)
	{
		return Slots.Num();
	}

	return -1; // Inventory is full
}
