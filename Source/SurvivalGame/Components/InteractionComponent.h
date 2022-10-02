// Fill out your copyright notice in the Description page of Projet Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "InteractionComponent.generated.h"

class ASurvivalCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeginInteract, ASurvivalCharacter*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndInteract, ASurvivalCharacter*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeginFocus, ASurvivalCharacter*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndFocus, ASurvivalCharacter*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteract, ASurvivalCharacter*, Character);

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SURVIVALGAME_API UInteractionComponent : public UWidgetComponent
{
	GENERATED_BODY()
	
public:
	UInteractionComponent();

	// Delegates

	//[local + server] Called when the player presses the interact key whilst focusing on this interactable actor
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnBeginInteract OnBeginInteract;

	//[[local + server] Called when the player releases the interact key, stops looking at the interactable actor, or gets too far away after starting an interact.
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnEndInteract OnEndInteract;

	//[local + server] Called when the player presses the interact key whilst focusing on this interactable actor.
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnBeginFocus OnBeginFocus;

	//[[local + server] Called when the player releases the interact key, stops looking at the interactable actor, or gets too far away after starting an interact.
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnEndFocus OnEndFocus;

	//[local + server] Called when the player has interacted with the item for the required amount of time
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnInteract OnInteract;
	
protected:
	virtual void Deactivate() override;

	bool CanInteract(ASurvivalCharacter* character) const;

	// On the server, this will hold all interactors. On the local player, this will just hold the local player (provided they are an interactor).
	UPROPERTY()
	TArray<ASurvivalCharacter*> Interactors;

public:
	/** Refresh the interaction widget and its custom widgets.
	  * An example of when we'd use this is when we take 3 items out of a stack of 10, and we need the update the widget
	  * so it shows the stack as having 7 ites left. 
	  **/
	void RefreshWidget();

	void BeginFocus(ASurvivalCharacter* character);
	void EndFocus(ASurvivalCharacter* character);

	void BeginInteract(ASurvivalCharacter* character);
	void EndInteract(ASurvivalCharacter* character);

	void Interact(ASurvivalCharacter* character);

	// Return a value from 0-1 denoting how far through the interact we are.
	// On server this is the first interactors percentage, on client this is the local interactors percentage.
	UFUNCTION(BlueprintPure, Category = "Interaction")
	float GetInteractPercentage();

	FORCEINLINE float GetInteractionDistance() const { return InteractionDistance; }
	FORCEINLINE float GetInteractionTime() const { return InteractionTime; }

	FORCEINLINE void SetInteractionTime(float interactionTime) { InteractionTime = interactionTime; }
	FORCEINLINE void SetInteractionDistance(float interactionDistance) { InteractionDistance = interactionDistance; }
	
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractableNameText(const FText& NewNameText);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractableActionText(const FText& NewActionText);

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
	float InteractionTime;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
	float InteractionDistance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
	FText InteractableNameText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
	FText InteractableActionText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
	bool bAllowMultipleInteractors;
};
