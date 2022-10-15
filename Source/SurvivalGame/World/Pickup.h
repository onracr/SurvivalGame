// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

UCLASS()
class SURVIVALGAME_API APickup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickup();

	// Takes the item to represent and creates the pickup from it. Done on BeginPlay() and when a player drops an item on the ground.
	void InitializePickup(const TSubclassOf<class UItem> ItemClass, const int32 Quantity);

	// Align pickups rotation with ground rotation.
	UFUNCTION(BlueprintImplementableEvent)
	void AlignWithGround();

	UItem* GetItem() const { return Item; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// Called when a player takes the pickup
	UFUNCTION()
	void OnTakePickup(class ASurvivalCharacter* Taker);

	UFUNCTION()
	void OnRep_Item();

	/** If some property on the item is modified, we bind this to OnItemModified and refresh the UI if the item gets modified. */
	UFUNCTION()
	void OnItemModified();

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced, meta = (AllowPrivateAccess = true))
	UItem* ItemTemplate;

	// This is used as a template to create the pickup when spawned in.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_Item, meta = (AllowPrivateAccess = true))
	UItem* Item;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = true))
	class UStaticMeshComponent* PickupMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = true))
	class UInteractionComponent* InteractionComponent;
};
