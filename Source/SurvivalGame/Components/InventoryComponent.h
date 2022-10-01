// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

// Called when the inventory is changed and the UI needs an update.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);

UENUM(BlueprintType)
enum class EItemAddResult : uint8
{
	IAR_NOITEMSADDED	UMETA(DisplayName = "No Items Added"),
	IAR_SOMEITEMSADDED	UMETA(DisplayName = "Some items added"),
	IAR_ALLITEMSADDED	UMETA(DisplayName = "All items added")
};

/** Represents the result of adding an item to the inventory. */
USTRUCT(BlueprintType)	// this can be used in blueprint as well as in cpp.
struct FItemAddResult
{
	GENERATED_BODY()

public:
	FItemAddResult() = default;
	FItemAddResult(int32 InItemQuantity) : AmountToGive(InItemQuantity), ActualAmountGiven(0) {};
	FItemAddResult(int32 InItemQuantity, int32 InQuantityAdded) : AmountToGive(InItemQuantity), ActualAmountGiven(InQuantityAdded) {};

public:
	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	int32 AmountToGive;

	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	int32 ActualAmountGiven;

	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	EItemAddResult Result;

	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	FText ErrorText;

	// Helpers
	static FItemAddResult AddedNone(const int32 InItemQuantity, const FText& ErrorText)
	{
		FItemAddResult AddedNoneResult(InItemQuantity);
		
		AddedNoneResult.Result = EItemAddResult::IAR_NOITEMSADDED;

		return AddedNoneResult;
	}
	static FItemAddResult AddedSome(const int32 InItemQuantity, const int32 ActualAmountGiven, const FText& ErrorText)
	{
		FItemAddResult AddedSomeResult(InItemQuantity, ActualAmountGiven);

		AddedSomeResult.Result = EItemAddResult::IAR_SOMEITEMSADDED;
		AddedSomeResult.ErrorText = ErrorText;

		return AddedSomeResult;
	}
	static FItemAddResult AddedAll(const int32 InItemQuantity)
	{
		FItemAddResult AddAllResult(InItemQuantity, InItemQuantity);

		AddAllResult.Result = EItemAddResult::IAR_ALLITEMSADDED;

		return AddAllResult;
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SURVIVALGAME_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

	friend class UItem;

public:	
	// Sets default values for this component's properties
	UInventoryComponent();

public:	
	
	/** Add an item to the inventory.
	@param ErrorText the text to display if the item couldn't be added to the inventory.
	@return the amount of the item was added to the inventory. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FItemAddResult TryAddItem(UItem* Item);

	/** Add an item to the inventory using the item class instead of an item instance.
	@param ErrorText the text to display if the item couldn't be added to the inventory.
	@return the amount of the item that was added to the inventory. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FItemAddResult TryAddItemFromClass(TSubclassOf<class UItem> ItemClass, const int32 Quantity);

	/** Take some quantity away from the item, and remove it from the inventory when quantity reaches zero. 
	Useful for actions like eating food, using ammo, etc. */

	int32 ConsumeItem(UItem* Item);
	int32 ConsumeItem(UItem* Item, const int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = Inventory)
	bool RemoveItem(UItem* Item);

	UFUNCTION(BlueprintPure, Category = Inventory)
	bool HasItem(TSubclassOf<UItem> ItemClass, const int32 Quantity = 1) const;

	UFUNCTION(BlueprintPure, Category = Inventory)
	UItem* FindItem(UItem* Item) const;

	UFUNCTION(BlueprintPure, Category = Inventory)
	UItem* FindItemByClass(TSubclassOf<UItem> ItemClass) const;

	UFUNCTION(BlueprintPure, Category = Inventory)
	TArray<UItem*> FindItemsByClass(TSubclassOf<UItem> ItemClass) const;

	UFUNCTION(BlueprintPure, Category = Inventory)
	float GetCurrentWeight() const;

	UFUNCTION(BLueprintCallable, Category = Inventory)
	void SetWeightCapacity(const float NewWeightCapacity);

	UFUNCTION(BlueprintCallable, Category = Inventory)
	void SetCapacity(const int32 NewCapacity);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE float GetWeightCapacity() const { return WeightCapacity; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE int32 GetCapacity() const { return Capacity; }

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE TArray<class UItem*> GetItems() const { return Items; }

	UFUNCTION(Client, Reliable)
	void ClientRefreshInventory();

	UPROPERTY(BlueprintAssignable, Category = Inventory)
	FOnInventoryUpdated OnInventoryUpdated;

protected:

	// Maximum weight the inventory can hold. For players, backpacks and other items can increase this limit.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory")
	float WeightCapacity;

	// Maximum number of items the inventory can hold. For players, backpacks and other items can increase this limit.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory", meta = (ClampMin = 0, ClampMax = 200))
	int32 Capacity /* : 2 */;

	UPROPERTY(ReplicatedUsing = OnRep_Items, VisibleAnywhere, Category = "Inventory")
	TArray<class UItem*> Items;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags);

private:
	/** Don't call Items.Add() directly, use this function instead, as it handles replication and ownership. */
	UItem* AddItem(UItem* Item);

	UFUNCTION()
	void OnRep_Items();	// if we get or lose an item, inventory UI will be refreshed.
		
	// each item have RepKey and inventory have also a repkey to check if items array is changed!
	UPROPERTY()
	int32 ReplicatedItemsKey;	// just a number that changes when items need to replicate!

	// Internal, non-BP exposed add item function. Don't call this directly,; use TryAddItem(), or TryAddItemfromClass() instead.
	FItemAddResult TryAddItem_Internal(UItem* Item);
};
