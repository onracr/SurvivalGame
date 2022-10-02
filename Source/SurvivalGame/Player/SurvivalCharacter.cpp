 // Fill out your copyright notice in the Description page of Project Settings.


#include "SurvivalCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InteractionComponent.h"
#include "Components/InventoryComponent.h"

// Sets default values
ASurvivalCharacter::ASurvivalCharacter() :
	InteractionCheckFrequency(0.f), InteractionCheckDistance(1000.f)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(FName(TEXT("Camera")));
	Camera->SetupAttachment(GetMesh(), FName("CameraSocket"));
	Camera->bUsePawnControlRotation = true;

	HelmetMesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName(TEXT("HelmetMesh")));
	HelmetMesh->SetupAttachment(GetMesh());
	HelmetMesh->SetMasterPoseComponent(GetMesh());

	ChestMesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName(TEXT("ChestMesh")));
	ChestMesh->SetupAttachment(GetMesh());
	ChestMesh->SetMasterPoseComponent(GetMesh());

	LegsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName(TEXT("LegsMesh")));
	LegsMesh->SetupAttachment(GetMesh());
	LegsMesh->SetMasterPoseComponent(GetMesh());
	
	FeetMesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName(TEXT("FeetMesh")));
	FeetMesh->SetupAttachment(GetMesh()); 
	FeetMesh->SetMasterPoseComponent(GetMesh());
	
	VestMesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName(TEXT("VestMesh")));
	VestMesh->SetupAttachment(GetMesh());
	VestMesh->SetMasterPoseComponent(GetMesh());
	
	HandsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName(TEXT("HandsMesh")));
	HandsMesh->SetupAttachment(GetMesh());
	HandsMesh->SetMasterPoseComponent(GetMesh());
	
	BackpackMesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName(TEXT("BackpackMesh")));
	BackpackMesh->SetupAttachment(GetMesh());
	BackpackMesh->SetMasterPoseComponent(GetMesh());

	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->bCastHiddenShadow = true;
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	
	PlayerInventory = CreateDefaultSubobject<UInventoryComponent>(FName(TEXT("Inventory")));
	PlayerInventory->SetCapacity(20);
	PlayerInventory->SetWeightCapacity(60.f);
}

// Called when the game starts or when spawned
void ASurvivalCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASurvivalCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const bool bIsInteractingOnServer = (HasAuthority() && IsInteracting());

	if ((!HasAuthority() || bIsInteractingOnServer) && GetWorld()->TimeSince(InteractionData.LastInteractionCheckTime) > InteractionCheckFrequency)
		PerformInteractionCheck();
}

// Called to bind functionality to input
void ASurvivalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ASurvivalCharacter::BeginInteract);
	PlayerInputComponent->BindAction("Interact", IE_Released, this, &ASurvivalCharacter::EndInteract);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASurvivalCharacter::StartCrouching);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASurvivalCharacter::StopCrouching);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASurvivalCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASurvivalCharacter::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &ASurvivalCharacter::LookUp);
	PlayerInputComponent->BindAxis("Turn", this, &ASurvivalCharacter::Turn);
}

void ASurvivalCharacter::MoveForward(float value)
{
	if (value != 0.f)
		AddMovementInput(GetActorForwardVector(), value);
}

void ASurvivalCharacter::MoveRight(float value)
{
	if (value != 0.f)
		AddMovementInput(GetActorRightVector(), value);
}

void ASurvivalCharacter::LookUp(float value)
{
	if (value != 0.f)
		AddControllerPitchInput(value);
}

void ASurvivalCharacter::Turn(float value)
{
	if (value != 0.f)
		AddControllerYawInput(value);
}

void ASurvivalCharacter::StartCrouching()
{
	Crouch();
}

void ASurvivalCharacter::StopCrouching()
{
	UnCrouch();
}

void ASurvivalCharacter::PerformInteractionCheck()
{
	if (!GetController()) return;

	InteractionData.LastInteractionCheckTime = GetWorld()->GetTimeSeconds();

	FVector EyesLocation;
	FRotator EyesRotation;

	GetController()->GetPlayerViewPoint(EyesLocation, EyesRotation);

	FVector TraceStart = EyesLocation;
	FVector TraceEnd = EyesRotation.Vector() * InteractionCheckDistance + TraceStart;
	FHitResult TraceHit;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(TraceHit, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, QueryParams))
	{
		if (TraceHit.GetActor())
		{
			if (UInteractionComponent* InteractionComponent = Cast<UInteractionComponent>(TraceHit.GetActor()->GetComponentByClass(UInteractionComponent::StaticClass())))
			{
				float distance = (TraceStart - TraceHit.ImpactPoint).Size();

				if (InteractionComponent != GetInteractable() && distance <= InteractionComponent->GetInteractionDistance())
				{
					FoundNewInteractable(InteractionComponent);
				}
				else if (distance > InteractionComponent->GetInteractionDistance() && GetInteractable())
				{
					CouldntFindInteractable();
				}

				return;
			}
			else
			{
				CouldntFindInteractable();
			}
		}
	}
}

void ASurvivalCharacter::CouldntFindInteractable()
{
	if (GetWorldTimerManager().IsTimerActive(TimerHandle_Interact))
	{
		GetWorldTimerManager().ClearTimer(TimerHandle_Interact);
	}

	if (UInteractionComponent* Interactable = GetInteractable())
	{
		Interactable->EndFocus(this);

		if (InteractionData.bInteractHeld)
		{
			EndInteract();
		}
	}
	InteractionData.ViewedInteractionComponent = nullptr;
}

void ASurvivalCharacter::FoundNewInteractable(UInteractionComponent* Interactable)
{
	EndInteract();

	if (UInteractionComponent* oldInteractable = GetInteractable())
	{
		oldInteractable->EndFocus(this);
	}
	InteractionData.ViewedInteractionComponent = Interactable;
	Interactable->BeginFocus(this);
}

void ASurvivalCharacter::BeginInteract()
{
	if (!HasAuthority())
		ServerBeginInteract();

	/** As an optimization, server only checks that we're looking at an item once we begin interacting with it. 
	 * This saves the server doing a check every tick for an interactable Item. The exception is a non-instant
	 * interact. In this case, the server will check every tick for the duration of the interact. 
	 **/
	if (HasAuthority())
	{
		PerformInteractionCheck();
	}

	InteractionData.bInteractHeld = true;

	if (UInteractionComponent* interactable = GetInteractable())
	{
		interactable->BeginInteract(this);

		if (FMath::IsNearlyZero(interactable->GetInteractionTime()))
		{
			Interact();
		}
		else
		{
			GetWorldTimerManager().SetTimer(TimerHandle_Interact, this, &ASurvivalCharacter::Interact, interactable->GetInteractionTime(), false);
		}
	}
}

void ASurvivalCharacter::EndInteract()
{
	if (!HasAuthority())
		ServerEndInteract();

	InteractionData.bInteractHeld = false;

	GetWorldTimerManager().ClearTimer(TimerHandle_Interact);

	if (UInteractionComponent* interactable = GetInteractable())
	{
		interactable->EndInteract(this);
	}
}

void ASurvivalCharacter::Interact()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_Interact);
	if (UInteractionComponent* Interactable = GetInteractable())
	{
		Interactable->Interact(this);		
	}
}

bool ASurvivalCharacter::IsInteracting() const
{
	return GetWorldTimerManager().IsTimerActive(TimerHandle_Interact);
}

float ASurvivalCharacter::GetRemainingInteractTime() const
{
	return GetWorldTimerManager().GetTimerRemaining(TimerHandle_Interact);
}

void ASurvivalCharacter::ServerBeginInteract_Implementation()
{
	BeginInteract();
}

bool ASurvivalCharacter::ServerBeginInteract_Validate()
{
	return true;
}

void ASurvivalCharacter::ServerEndInteract_Implementation()
{
	EndInteract();
}

bool ASurvivalCharacter::ServerEndInteract_Validate()
{
	return true;
}