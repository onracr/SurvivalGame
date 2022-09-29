// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "FoodItem.generated.h"

/**
 * 
 */
UCLASS()
class SURVIVALGAME_API UFoodItem : public UItem
{
	GENERATED_BODY()
	
public:
	UFoodItem();

	virtual void Use(class ASurvivalCharacter* character) override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Healing", meta = (AllowPrivateAccess = "true"))
	float HealAmount;
};
