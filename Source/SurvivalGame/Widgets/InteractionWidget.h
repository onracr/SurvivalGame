// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractionWidget.generated.h"

class UInteractionComponent;

/**
 * 
 */
UCLASS()
class SURVIVALGAME_API UInteractionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void UpdateInteractionWidget(UInteractionComponent* InteractionComponent);
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnUpdateInteractionWidget();

private:
	UPROPERTY(BlueprintReadOnly, Category = "Interaction", meta = (ExposeOnSpawn, AllowPrivateAccess = "true"))
	UInteractionComponent* OwningInteractionComponent;
};
