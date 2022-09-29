// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryComponent.h"
#include "Items/Item.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h" // to replicate UObjects

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	SetIsReplicated(true);	// other players will see each others' inventory etc. 
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
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

void UInventoryComponent::OnRep_Items()
{

}
