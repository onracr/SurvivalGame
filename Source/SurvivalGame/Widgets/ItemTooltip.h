// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemTooltip.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALGAME_API UItemTooltip : public UUserWidget
{
	GENERATED_BODY()

private:
	UPROPERTY(BlueprintReadOnly, Category = "Tooltip", meta = (ExposeOnSpawn = true, AllowPrivateAccess = true))
	class UInventoryItemWidget* InventoryItemWidget;
	
};
