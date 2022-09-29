// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InteractionComponent.h"
#include "Player/SurvivalCharacter.h"
#include "Widgets/InteractionWidget.h"

UInteractionComponent::UInteractionComponent() :
	InteractionTime(0.f), InteractionDistance(200.f), InteractableNameText(FText::FromString("Interactable Object")),
	InteractableActionText(FText::FromString("Interact")), bAllowMultipleInteractors(true)
{
	SetComponentTickEnabled(false);

	Space = EWidgetSpace::Screen;
	DrawSize = FIntPoint(600, 100);
	bDrawAtDesiredSize = true;

	SetActive(true);
	SetHiddenInGame(true);
}

void UInteractionComponent::Deactivate()
{
	Super::Deactivate();

	for (int32 i = Interactors.Num() - 1; i >= 0; --i)
	{
		if (ASurvivalCharacter* Interactor = Interactors[i])
		{
			EndFocus(Interactor);
			EndInteract(Interactor);
		}
	}
	Interactors.Empty();
}

bool UInteractionComponent::CanInteract(ASurvivalCharacter* character) const
{
	const bool bPlayerAlreadyInteracting = !bAllowMultipleInteractors && Interactors.Num() >= 1;

	return !bPlayerAlreadyInteracting && IsActive() && GetOwner() != nullptr && character != nullptr;
}

void UInteractionComponent::RefreshWidget()
{
	if (!bHiddenInGame && GetOwner()->GetNetMode() != NM_DedicatedServer)
	{
		if (UInteractionWidget* InteractionWidget = Cast<UInteractionWidget>(GetUserWidgetObject()))
		{
			InteractionWidget->UpdateInteractionWidget(this);
		}
	}
}

void UInteractionComponent::BeginFocus(ASurvivalCharacter* character)
{
	if (!IsActive() || !GetOwner() || !character)
		return;

	OnBeginFocus.Broadcast(character);

	SetHiddenInGame(false);

	if (!GetOwner()->HasAuthority())
	{
		for (auto& visualComp : GetOwner()->GetComponentsByClass(UPrimitiveComponent::StaticClass()))
		{
			if (UPrimitiveComponent* primitive = Cast<UPrimitiveComponent>(visualComp))
			{
				primitive->SetRenderCustomDepth(true);
			}
		}
	}
	RefreshWidget();
}

void UInteractionComponent::EndFocus(ASurvivalCharacter* character)
{
	OnEndFocus.Broadcast(character);

	SetHiddenInGame(true);

	if (!GetOwner()->HasAuthority())
	{
		for (auto& visualComp : GetOwner()->GetComponentsByClass(UPrimitiveComponent::StaticClass()))
		{
			if (UPrimitiveComponent* primitive = Cast<UPrimitiveComponent>(visualComp))
			{
				primitive->SetRenderCustomDepth(false);
			}
		}
	}
}

void UInteractionComponent::BeginInteract(ASurvivalCharacter* character)
{
	if (CanInteract(character))
	{
		Interactors.AddUnique(character);
		OnBeginInteract.Broadcast(character);
	}
}

void UInteractionComponent::EndInteract(ASurvivalCharacter* character)
{
	Interactors.RemoveSingle(character);
	OnEndInteract.Broadcast(character);
}

void UInteractionComponent::Interact(ASurvivalCharacter* character)
{
	if (CanInteract(character))
		OnInteract.Broadcast(character);
}

float UInteractionComponent::GetInteractPercentage()
{
	if (Interactors.IsValidIndex(0)) // reason the index of being 0 is that (player)character'll always be index 0!
	{
		if (ASurvivalCharacter* Interactor = Interactors[0])
		{
			if (Interactor && Interactor->IsInteracting())
			{
				return 1.f - FMath::Abs(Interactor->GetRemainingInteractTime() / InteractionTime);				
			}
		}
	}
	return 0.0f;
}

void UInteractionComponent::SetInteractableNameText(const FText& NewNameText)
{
	InteractableNameText = NewNameText;
	RefreshWidget();
}

void UInteractionComponent::SetInteractableActionText(const FText& NewActionText)
{
	InteractableActionText = NewActionText;
	RefreshWidget();
}
