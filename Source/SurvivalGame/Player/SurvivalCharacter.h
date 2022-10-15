// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SurvivalCharacter.generated.h"

class UInteractionComponent;
class UInventoryComponent;

USTRUCT()
struct FInteractionData
{
	GENERATED_BODY()

	FInteractionData()
	{
		ViewedInteractionComponent = nullptr;
		LastInteractionCheckTime = 0.f;
		bInteractHeld = false;
	}

	UPROPERTY()
	UInteractionComponent* ViewedInteractionComponent;

	UPROPERTY()
	float LastInteractionCheckTime;

	// whether local player is holding the interact key
	UPROPERTY()
	bool bInteractHeld;
};

UCLASS()
class SURVIVALGAME_API ASurvivalCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASurvivalCharacter();

	
protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	/* Interactable helper Functions */
	void PerformInteractionCheck();

	void CouldntFindInteractable();
	void FoundNewInteractable(UInteractionComponent* Interactable);

	void BeginInteract();
	void EndInteract();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerBeginInteract();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEndInteract();

	UFUNCTION(BlueprintCallable, Category = Items)
	void UseItem(UItem* Item);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerUseItem(UItem* Item);

	UFUNCTION(BlueprintCallable, Category = Items)
	void DropItem(UItem* Item, const int32 Quantity);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerDropItem(UItem* Item, const int32 Quantity);

	void Interact();
	bool IsInteracting() const;
	float GetRemainingInteractTime() const;

	FORCEINLINE UInteractionComponent* GetInteractable() const { return InteractionData.ViewedInteractionComponent; }

	UFUNCTION()
	FORCEINLINE UInventoryComponent* GetPlayerInventory() const { return PlayerInventory; }

protected:
	void MoveForward(float value);
	void MoveRight(float value);
	void LookUp(float value);
	void Turn(float value);

	void StartCrouching();
	void StopCrouching();

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = true))
	UInventoryComponent* PlayerInventory;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compoenent", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compoenent", meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* HelmetMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compoenent", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ChestMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compoenent", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* LegsMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compoenent", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FeetMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compoenent", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* VestMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compoenent", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* HandsMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compoenent", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* BackpackMesh;

	// We need this because the pickups use a blueprint base class.
	UPROPERTY(EditDefaultsOnly, Category = Item, meta = (AllowPrivateAccess = true))
	TSubclassOf<class APickup> PickupClass;

	// How often in seconds to check an interactable object. Set this to zero if you want to check every tick.
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float InteractionCheckFrequency;

	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float InteractionCheckDistance;

	FTimerHandle TimerHandle_Interact;

	UPROPERTY()
	FInteractionData InteractionData;
	
	// 18. derste kaldim! 16 tekrar
};
