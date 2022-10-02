// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryComponent.h"
#include "Items/Item.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h" // to replicate UObjects

#define LOCTEXT_NAMESPACE "Inventory"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	SetIsReplicated(true);	// other players will see each others' inventory etc. 
}

FItemAddResult UInventoryComponent::TryAddItem(UItem* Item)
{
	return TryAddItem_Internal(Item);
}

FItemAddResult UInventoryComponent::TryAddItemFromClass(TSubclassOf<class UItem> ItemClass, const int32 Quantity)
{
	UItem* Item = NewObject<UItem>(GetOwner(), ItemClass);
	Item->SetQuantity(Quantity);

	return TryAddItem_Internal(Item);
}

int32 UInventoryComponent::ConsumeItem(UItem* Item)
{
	if (Item)
	{
		ConsumeItem(Item, Item->GetQuantity());
	}
	return 0;
}

int32 UInventoryComponent::ConsumeItem(UItem* Item, const int32 Quantity)
{
	if (Item && GetOwner() && GetOwner()->HasAuthority())
	{
		const int32 RemoveQuantity = FMath::Min(Quantity, Item->GetQuantity());

		// we shouldn't have a negative amount of the item after the drop
		ensure(!(Item->GetQuantity() - RemoveQuantity < 0));

		// we now have zero of this item, remove from the inventory
		Item->SetQuantity(Item->GetQuantity() - RemoveQuantity);

		if (Item->GetQuantity() <= 0)
		{
			RemoveItem(Item);
		}
		else
		{
			ClientRefreshInventory();
		}
	}
	return 0;
}

bool UInventoryComponent::RemoveItem(UItem* Item)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (Item)
		{
			Items.RemoveSingle(Item);
			ReplicatedItemsKey++;

			return true;
		}
	}

	return false;
}

bool UInventoryComponent::HasItem(TSubclassOf<UItem> ItemClass, const int32 Quantity) const
{
	if (UItem* ItemToFind = FindItemByClass(ItemClass))
	{
		return ItemToFind->GetQuantity() >= Quantity;
	}
	return false;
}

UItem* UInventoryComponent::FindItem(UItem* Item) const
{
	if (Item)
	{
		for (auto& InvItem : Items)
		{
			if (InvItem && InvItem->GetClass() == Item->GetClass())
			{
				return InvItem;
			}
		}
	}
	return nullptr;
}

UItem* UInventoryComponent::FindItemByClass(TSubclassOf<UItem> ItemClass) const
{
	for (auto& InvItem : Items)
	{
		if (InvItem && InvItem->GetClass() == ItemClass)
		{
			return InvItem;
		}
	}
	return nullptr;
}

TArray<UItem*> UInventoryComponent::FindItemsByClass(TSubclassOf<UItem> ItemClass) const
{
	TArray<UItem*> ItemsOfClass;

	for (auto& InvItem : Items)
	{
		if (InvItem && InvItem->GetClass()->IsChildOf(ItemClass))
		{
			ItemsOfClass.Add(InvItem);
		}
	}
	return ItemsOfClass;
}

float UInventoryComponent::GetCurrentWeight() const
{
	float weight = 0.f;

	for (auto& Item : Items)
	{
		if (Item)
		{
			weight += Item->GetStackWeight();
		}
	}

	return weight;
}

void UInventoryComponent::SetWeightCapacity(const float NewWeightCapacity)
{
	WeightCapacity = NewWeightCapacity;
	OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::SetCapacity(const int32 NewCapacity)
{
	Capacity = NewCapacity;
	OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::ClientRefreshInventory_Implementation()
{
	OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, Items);
}

bool UInventoryComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags); // whether or not we wrote something to actor channel

	/* Typically for anything that isn't a UObject, just using DOREPLIFETIME along with UPROPERTY(Replicated)
	 * is all we need!
	 * We don't need to replicate all things, only changes!
	 */
	
	// Check if the array of items needs to replicate
	if (Channel->KeyNeedsToReplicate(0, ReplicatedItemsKey))
	{
		for (auto& Item : Items)
		{
			if (Channel->KeyNeedsToReplicate(Item->GetUniqueID(), Item->RepKey))	// lesson 13.
			{
				bWroteSomething |= Channel->ReplicateSubobject(Item, *Bunch, *RepFlags);
			}
		}
	}
	return bWroteSomething;
}

UItem* UInventoryComponent::AddItem(UItem* Item)
{
	auto Owner = GetOwner();
	if (Owner && Owner->HasAuthority())	// owner is the actor that has this component.
	{
		UItem* NewItem = NewObject<UItem>(Owner, Item->GetClass());	// Reconstructing the Item and owning this Item.
		NewItem->SetQuantity(Item->GetQuantity());
		NewItem->SetOwningInventory(this);
		NewItem->AddedToInventory(this);
		Items.Add(NewItem);
		NewItem->MarkDirtyForReplication();

		return NewItem;
	}

	return nullptr;
}

void UInventoryComponent::OnRep_Items()
{
	OnInventoryUpdated.Broadcast();
}

FItemAddResult UInventoryComponent::TryAddItem_Internal(UItem* Item)
{
	auto Owner = GetOwner();
	if (Owner && Owner->HasAuthority())
	{
		const int32 AddAmount = Item->GetQuantity();

		if (Items.Num() + 1 > GetCapacity())
		{
			return FItemAddResult::AddedNone(AddAmount, LOCTEXT("InventoryCapacityFullText", "Couldn't add item to Inventory. Inventory is full."));
		}

		// Items with a weight of zero don't require a weight check
		if (!FMath::IsNearlyZero(Item->GetWeight()))
		{
			if (GetCurrentWeight() + Item->GetWeight() > GetWeightCapacity())
			{
				FItemAddResult::AddedNone(AddAmount, LOCTEXT("InventoryTooMuchWeightText", "Couldn't add item to Inventory. Carrying too much weight."));
			}
		}

		// If the item is stackable, check if we already have it and add it to their stack
		if (Item->GetIsStackable())
		{
			ensure(Item->GetQuantity() <= Item->GetMaxStackSize());

			if (UItem* ExistingItem = FindItem(Item))
			{
				if (ExistingItem->GetQuantity() < ExistingItem->GetMaxStackSize())
				{
					// Find out how much item can be added more
					const int32 CapacityMaxAddAmount = ExistingItem->GetMaxStackSize() - ExistingItem->GetQuantity();
					int32 ActualAddAmount = FMath::Min(AddAmount, CapacityMaxAddAmount);

					FText ErrorText = LOCTEXT("InventoryErrorText", "Couldn't add al of the item to your inventory.");

					// Adjust based on how much weight we can carry.
					if (!FMath::IsNearlyZero(Item->GetWeight()))
					{
						// Find the maximum amount of the item we could take due to weight
						const int32 WeightMaxAddAmount = FMath::FloorToInt((WeightCapacity - GetCurrentWeight()) / Item->GetWeight());
						ActualAddAmount = FMath::Min(ActualAddAmount, WeightMaxAddAmount);

						if (ActualAddAmount < AddAmount)
						{
							ErrorText = FText::Format(LOCTEXT("InventoryTooMuchWeightText", "Couldn't add entire stack of {ItemName} to Inventory."), Item->GetItemDisplayName());
						}
					}
					else if (ActualAddAmount < AddAmount)
					{
						// If the item weights none and we can't take it, then there was a capacity issue.
						ErrorText = FText::Format(LOCTEXT("InventoryCapacityFullText", "Couldn't add entire stack of {ItemName} to Inventory. Inventory is full."), Item->GetItemDisplayName());
					}

					// We couldn't add any of the item to our inventory.
					if (ActualAddAmount <= 0)
					{
						return FItemAddResult::AddedNone(AddAmount, LOCTEXT("InventoryErrorText", "Couldn't add item to Inventory."));
					}

					ExistingItem->SetQuantity(ExistingItem->GetQuantity() + ActualAddAmount);

					// If we somehow get more of the item than the max stack size then something is wrong with our math.
					ensure(ExistingItem->GetQuantity() <= ExistingItem->GetMaxStackSize());

					if (ActualAddAmount < AddAmount)
					{
						return FItemAddResult::AddedSome(AddAmount, ActualAddAmount, ErrorText);
					}
					else
					{
						return FItemAddResult::AddedAll(AddAmount);
					}
				}
				else
				{
					return FItemAddResult::AddedNone(AddAmount, FText::Format(LOCTEXT("InventoryFullStackText", "Couldn't add {ItemName}. You already have a full stack of this Item."), Item->GetItemDisplayName()));
				}
			}
			else
			{
				// Since we don't have any of this item, we'll add the full stack.
				AddItem(Item);
				return FItemAddResult::AddedAll(AddAmount);
			}
		}
		else // item is not stackable
		{
			// None-stackables should always have a quantity of 1.
			ensure(Item->GetQuantity() == 1);

			AddItem(Item);
			return FItemAddResult::AddedAll(AddAmount);
		}
	}
	
	// AddItem should never be called on a client
	check(false);
	return FItemAddResult::AddedNone(-1, LOCTEXT("ErrorMessage", ""));

}

#undef LOCTEXT_NAMESPACE