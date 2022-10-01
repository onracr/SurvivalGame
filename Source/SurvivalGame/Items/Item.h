// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Item.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnItemModified);

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	IR_COMMON		UMETA(DisplayName = "Common"),
	IR_UNCOMMON		UMETA(DisplayName = "Uncommon"),
	IR_RARE			UMETA(DisplayName = "Rare"),
	IR_VERYRARE		UMETA(DisplayName = "VeryRare"),
	IR_LEGENDARY	UMETA(DisplayName = "Legendary")
};

/**
 * 
 */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class SURVIVALGAME_API UItem : public UObject
{
	GENERATED_BODY()

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	UItem();

	UFUNCTION(BlueprintCallable, Category = "Item")
	FORCEINLINE float GetStackWeight() const { return Quantity * Weight; }
	FORCEINLINE FText GetItemDisplayName() const { return ItemDisplayName; }
	FORCEINLINE float GetWeight() const { return Weight; }
	FORCEINLINE bool GetIsStackable() const { return bStackable; }
	FORCEINLINE int32 GetMaxStackSize() const { return MaxStackSize; }

	FORCEINLINE void SetOwningInventory(class UInventoryComponent* InventoryComponent)
	{
		OwningInventory = InventoryComponent;
	}

	UFUNCTION(BlueprintPure, Category = "Item")
	virtual bool ShouldShowInInventory() const;

	virtual void Use(class ASurvivalCharacter* character);
	virtual void AddedToInventory(UInventoryComponent* Inventory);

	// Marks the object as needing replication. We must call this internally after modifying any replicated properties
	void MarkDirtyForReplication();

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (AllowPrivateAccess = "true"))
	class UStaticMesh* PickupMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"))
	class UTexture2D* Thumbnail;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (AllowPrivateAccess = "true"))
	FText ItemDisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (MultiLine = "true", AllowPrivateAccess = "true"))
	FText ItemDescription;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (AllowPrivateAccess = "true"))
	FText UseActionText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (AllowPrivateAccess = "true"))
	EItemRarity ItemRarity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (Clamp = 0.0, AllowPrivateAccess = "true"))
	float Weight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (AllowPrivateAccess = "true"))
	bool bStackable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (ClampMin = 2, EditCondition = bStackable, AllowPrivateAccess = "true"))
	int32 MaxStackSize;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UItemTooltip> ItemTooltip;

	// Server manages this value
	UPROPERTY(ReplicatedUsing = OnRep_Quantity, EditAnywhere, Category = "Item", meta = (UIMin = 1, EditCondition = bStackable, AllowPrivateAccess = "true"))
	int32 Quantity;

	// The Inventory that owns this item
	UPROPERTY()
	UInventoryComponent* OwningInventory;

	UPROPERTY(BlueprintAssignable)
	FOnItemModified OnItemModified;

public:
	
	// Used to efficiently replicate inventory items
	UPROPERTY()
	int32 RepKey;

	UFUNCTION()
	void OnRep_Quantity();

	UFUNCTION(BlueprintCallable)
	void SetQuantity(const int32 NewQuantity);

	UFUNCTION(BlueprintPure, Category = "Item")
	FORCEINLINE int32 GetQuantity() const { return Quantity; }
};
