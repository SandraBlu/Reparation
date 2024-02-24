// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AOPlayerBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/AOInteractComponent.h"
#include "Components/AOInventoryComponent.h"
#include "Item/AOItem.h"

AAOPlayerBase::AAOPlayerBase()
{
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCam = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCam"));
	FollowCam->SetupAttachment(CameraBoom);

	//HeadAccessoryMesh = PlayerMeshes.Add(EEquipSlot::EIS_HeadAccessory, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HeadAccessoryMesh")));
	//TorsoMesh = PlayerMeshes.Add(EEquipSlot::EIS_Torso, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TorsoMesh")));
	//LegsMesh = PlayerMeshes.Add(EEquipSlot::EIS_Legs, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LegsMesh")));
	//HandsMesh = PlayerMeshes.Add(EEquipSlot::EIS_Hands, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HandsMesh")));
//	FeetMesh = PlayerMeshes.Add(EEquipSlot::EIS_Feet, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FeetMesh")));
	//BackpackMesh = PlayerMeshes.Add(EEquipSlot::EIS_Backpack, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BackpackMesh")));

	//Leader Pose Component
	/*for (auto& PlayerMesh : PlayerMeshes)
	{
		USkeletalMeshComponent* MeshComp = PlayerMesh.Value;
		MeshComp->SetupAttachment(GetMesh());
		MeshComp->SetLeaderPoseComponent(GetMesh());
	}*/
	//Add Head to Player mesh after Leader pose function so it does not attach self to self

	//PlayerMeshes.Add(EEquipSlot::EIS_Head, GetMesh());

	//Interact
	InteractCheckFrequency = 0.f;
	TraceDistance = 1000.f;

	//GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	//GetCharacterMovement()->MaxWalkSpeed = 500.f;
}

void AAOPlayerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	PerformInteractCheck();
}

void AAOPlayerBase::PerformInteractCheck()
{
	if (GetController() == nullptr)
	{
		return;
	}

	InteractData.LastInteractCheckTime = GetWorld()->GetTimeSeconds();

	FVector EyeLoc;
	FRotator EyeRot;
	GetController()->GetPlayerViewPoint(EyeLoc, EyeRot);

	FVector TraceStart = EyeLoc;
	FVector TraceEnd = (EyeRot.Vector() * TraceDistance) + TraceStart;
	TArray<FHitResult> TraceHits;

	FHitResult TraceHit;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(TraceHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{

		if (TraceHit.GetActor())
		{
			DrawDebugSphere(GetWorld(), TraceHit.ImpactPoint, 10.f, 32, FColor::Red, false, 0.0f);
			if (UAOInteractComponent* InteractComp = Cast<UAOInteractComponent>(TraceHit.GetActor()->GetComponentByClass(UAOInteractComponent::StaticClass())))
			{
				float Distance = (TraceStart - TraceHit.ImpactPoint).Size();
				if (InteractComp != GetInteractable() && Distance <= InteractComp->InteractDistance)
				{
					NewInteractableFound(InteractComp);
				}
				else if (Distance > InteractComp->InteractDistance && GetInteractable())
				{
					NoInteractableFound();
				}
				return;
			}
		}
	}
	NoInteractableFound();
}

void AAOPlayerBase::NoInteractableFound()
{
	if (GetWorldTimerManager().IsTimerActive(TimerHandle_Interact))
	{
		GetWorldTimerManager().ClearTimer(TimerHandle_Interact);
	}

	if (UAOInteractComponent* Interactable = GetInteractable())
	{
		Interactable->EndFocus(this);
		if (InteractData.bInteractHeld)
		{
			OnCompleteInteract();
		}
	}
	InteractData.ViewedInteractComp = nullptr;
}

void AAOPlayerBase::NewInteractableFound(class UAOInteractComponent* Interactable)
{
	UE_LOG(LogTemp, Warning, TEXT("Found an Interactable!!!"));
	OnCompleteInteract();

	if (UAOInteractComponent* OldInteractable = GetInteractable())
	{
		OldInteractable->EndFocus(this);
	}

	InteractData.ViewedInteractComp = Interactable;
	Interactable->BeginFocus(this);
}

void AAOPlayerBase::Interact()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_Interact);

	if (UAOInteractComponent* Interactable = GetInteractable())
	{
		Interactable->Interact(this);
	}
}

void AAOPlayerBase::OnStartInteract()
{
	InteractData.bInteractHeld = true;

	if (UAOInteractComponent* Interactable = GetInteractable())
	{
		Interactable->BeginInteract(this);
		if (FMath::IsNearlyZero(Interactable->InteractTime))
		{
			UE_LOG(LogTemp, Warning, TEXT("Interacting!!!"));
			Interact();
		}
		else
		{
			GetWorldTimerManager().SetTimer(TimerHandle_Interact, this, &AAOPlayerBase::Interact, Interactable->InteractTime, false);
		}
	}
}

void AAOPlayerBase::OnCompleteInteract()
{
	InteractData.bInteractHeld = false;

	GetWorldTimerManager().ClearTimer(TimerHandle_Interact);

	if (UAOInteractComponent* Interactable = GetInteractable())
	{
		Interactable->EndInteract(this);
	}
}

bool AAOPlayerBase::IsInteracting() const
{
	return GetWorldTimerManager().IsTimerActive(TimerHandle_Interact);
}

float AAOPlayerBase::GetRemainingInteractTime() const
{
	return GetWorldTimerManager().GetTimerRemaining(TimerHandle_Interact);
}

void AAOPlayerBase::UseItem(class UAOItem* Item)
{
	UE_LOG(LogTemp, Warning, TEXT("no item"));
	if (PlayerInventory && !PlayerInventory->FindItem(Item))
	{
		return;
	}

	if (Item)
	{
		Item->OnUse(this);
		Item->Use(this);
	}
}

void AAOPlayerBase::DrawWeapon()
{
	/*if (CanDisarm())
	{
		EquippedWeapon->PlayEquipMontage(FName("Disarm"));
		EquipState = EEquipState::EES_Unequipped;
		ActionState = EActionState::EAS_EquippingWeapon;
	}
	else if (CanDraw())
	{
		EquippedWeapon->PlayEquipMontage(FName("Draw"));
		EquipState = EEquipState::EES_EquippedMelee;
		ActionState = EActionState::EAS_EquippingWeapon;
	}*/
}

void AAOPlayerBase::EnterAttack()
{
	/*if (CanAttack())
	{
		ActionState = EActionState::EAS_Attacking;
	}*/
}

void AAOPlayerBase::UseThrowable()
{
	/*if (CanUseThrowable())
	{
		if (UThrowableItem* Throwable = GetThrowable())
		{
			PlayAnimMontage(Throwable->TossAnim);
			SpawnThrowable();

			if (PlayerInventory)
			{
				PlayerInventory->ConsumeItem(Throwable, 1);
			}
			else
			{
				if (Throwable->GetQuantity() <= 1)
				{
					EquippedItems.Remove(EEquipSlot::EIS_Throwable);
					OnEquippedItemsChanged.Broadcast(EEquipSlot::EIS_Throwable, nullptr);
				}
			}
		}
	}*/
}
