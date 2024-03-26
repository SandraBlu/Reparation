// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AOPlayerBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Components/AOInteractComponent.h"
#include "Components/AOInventoryComponent.h"
#include "Item/AOItem.h"
#include "Item/AOEquipItem.h"
#include "Item/AOGearItem.h"
#include "Item/AOThrowItem.h"
#include "Item/AOPickup.h"
#include "Item/AOWeaponItem.h"
#include "Weapon/AOWeapon.h"
#include "Animation/AnimMontage.h"
#include "Weapon/AOThrowable.h"
#include "Framework/AOPlayerState.h"
#include "Framework/AOPlayerController.h"
#include "UI/HUD/AOHUD.h"
#include "Abilities/AOAbilitySystemComponent.h"

#define LOCTEXT_NAMESPACE "AOCharacter"

AAOPlayerBase::AAOPlayerBase()
{
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCam = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCam"));
	FollowCam->SetupAttachment(CameraBoom);

	HeadAccessoryMesh = PlayerMeshes.Add(EEquipSlot::EIS_HeadAccessory, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HeadAccessoryMesh")));
	TorsoMesh = PlayerMeshes.Add(EEquipSlot::EIS_Torso, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TorsoMesh")));
	LegsMesh = PlayerMeshes.Add(EEquipSlot::EIS_Legs, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LegsMesh")));
	HandsMesh = PlayerMeshes.Add(EEquipSlot::EIS_Hands, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HandsMesh")));
	FeetMesh = PlayerMeshes.Add(EEquipSlot::EIS_Feet, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FeetMesh")));
	BackpackMesh = PlayerMeshes.Add(EEquipSlot::EIS_Backpack, CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BackpackMesh")));

	//Leader Pose Component
	for (auto& PlayerMesh : PlayerMeshes)
	{
		USkeletalMeshComponent* MeshComp = PlayerMesh.Value;
		MeshComp->SetupAttachment(GetMesh());
		MeshComp->SetLeaderPoseComponent(GetMesh());
	}
	//Add Head to Player mesh after Leader pose function so it does not attach self to self

	PlayerMeshes.Add(EEquipSlot::EIS_Head, GetMesh());

	//Interact
	InteractCheckFrequency = 0.f;
	TraceDistance = 1000.f;

	//GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	//GetCharacterMovement()->MaxWalkSpeed = 500.f;
}

void AAOPlayerBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	//Server
	InitAbilityActorInfo();
	GrantAbilities();
}

void AAOPlayerBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	//Client
	InitAbilityActorInfo();
}

AActor* AAOPlayerBase::GetCombatTarget_Implementation() const
{
	return CombatTarget;
}

void AAOPlayerBase::SetCombatTarget_Implementation(AActor* InCombatTarget)
{
	CombatTarget = InCombatTarget;
}

void AAOPlayerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	PerformInteractCheck();
}

void AAOPlayerBase::InitAbilityActorInfo()
{
	AAOPlayerState* AOPlayerState = GetPlayerState<AAOPlayerState>();
	check(AOPlayerState)
	AOPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(AOPlayerState, this);
	Cast<UAOAbilitySystemComponent>(AOPlayerState->GetAbilitySystemComponent())->AbilityActorInfoSet();
	AbilitySystemComponent = AOPlayerState->GetAbilitySystemComponent();
	AttributeSet = AOPlayerState->GetAttributeSet();

	if (AAOPlayerController* PC = Cast<AAOPlayerController>(GetController()))
	{
		if (AAOHUD* AOHUD = Cast<AAOHUD>(PC->GetHUD()))
		{
			{
				AOHUD->InitOverlay(PC, AOPlayerState, AbilitySystemComponent, AttributeSet);
			}
		}
	}
	InitializeAttributes();
}

int32 AAOPlayerBase::GetPlayerLevel()
{
	const AAOPlayerState* AOPlayerState = GetPlayerState<AAOPlayerState>();
	check(AOPlayerState)
	return AOPlayerState->GetPlayerLevel();
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

void AAOPlayerBase::OnWeaponEquipped()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->OnEquip();
		EquipState = EEquipState::EES_EquippedMelee;
	}
}

void AAOPlayerBase::SetWeaponCollision(ECollisionEnabled::Type CollisionEnabled)
{
	if (EquippedWeapon && EquippedWeapon->GetWeaponBox())
	{
		EquippedWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
		EquippedWeapon->IgnoreActors.Empty();
	}
}

void AAOPlayerBase::PlayTossFX(class UAnimMontage* TossMontage)
{
	PlayAnimMontage(TossMontage);
}

class UAOThrowItem* AAOPlayerBase::GetThrowable() const
{
	UAOThrowItem* EquippedThrowable = nullptr;

	if (EquippedItems.Contains(EEquipSlot::EIS_Throwable))
	{
		EquippedThrowable = Cast<UAOThrowItem>(*EquippedItems.Find(EEquipSlot::EIS_Throwable));
	}
	return EquippedThrowable;
}

void AAOPlayerBase::SpawnThrowable()
{
	if (UAOThrowItem* CurrentThrowable = GetThrowable())
	{
		if (CurrentThrowable->ThrowableClass)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = SpawnParams.Instigator = this;
			SpawnParams.bNoFail = true;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			FVector EyesLoc;
			FRotator EyesRot;
			GetController()->GetPlayerViewPoint(EyesLoc, EyesRot);
			//Move spawn point to slightly in front of player to avoid self collision
			EyesLoc = (EyesRot.Vector() * 20.f) + EyesLoc;

			if (AAOThrowable* ThrowableWeapon = GetWorld()->SpawnActor<AAOThrowable>(CurrentThrowable->ThrowableClass, FTransform(EyesRot, EyesLoc), SpawnParams))
			{
				PlayTossFX(CurrentThrowable->TossAnim);
			}

		}
	}
}

bool AAOPlayerBase::CanUseThrowable() const
{
	return GetThrowable() != nullptr && GetThrowable()->ThrowableClass != nullptr;
}

bool AAOPlayerBase::IsInteracting() const
{
	return GetWorldTimerManager().IsTimerActive(TimerHandle_Interact);
}

float AAOPlayerBase::GetRemainingInteractTime() const
{
	return GetWorldTimerManager().GetTimerRemaining(TimerHandle_Interact);
}

void AAOPlayerBase::DropItem(class UAOItem* Item, const int32 Quantity)
{
	if (PlayerInventory && Item && PlayerInventory->FindItem(Item))
	{
		const int32 ItemQuantity = Item->GetQuantity();
		const int32 DroppedQuantity = PlayerInventory->ConsumeItem(Item, Quantity);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.bNoFail = true;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		FVector SpawnLocation = GetActorLocation();
		SpawnLocation.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

		FTransform SpawnTransform(GetActorRotation(), SpawnLocation);

		ensure(PickupClass);

		AAOPickup* Pickup = GetWorld()->SpawnActor<AAOPickup>(PickupClass, SpawnTransform, SpawnParams);
		Pickup->InitializePickup(Item->GetClass(), DroppedQuantity);
	}
}

bool AAOPlayerBase::EquipItem(class UAOEquipItem* Item)
{
	EquippedItems.Add(Item->Slot, Item);
	OnEquippedItemsChanged.Broadcast(Item->Slot, Item);
	return true;
}

bool AAOPlayerBase::UnequipItem(class UAOEquipItem* Item)
{
	if (Item)
	{
		if (EquippedItems.Contains(Item->Slot))
		{
			if (Item == *EquippedItems.Find(Item->Slot))
			{
				EquippedItems.Remove(Item->Slot);
				OnEquippedItemsChanged.Broadcast(Item->Slot, nullptr);
				return true;
			}
		}
	}
	return false;
}

void AAOPlayerBase::EquipGear(class UAOGearItem* Gear)
{
	if (USkeletalMeshComponent* GearMesh = *PlayerMeshes.Find(Gear->Slot))
	{
		GearMesh->SetSkeletalMeshAsset(Gear->Mesh);
		GearMesh->SetMaterial(GearMesh->GetMaterials().Num() - 1, Gear->MaterialInstance);

	}
}

void AAOPlayerBase::UnequipGear(const EEquipSlot Slot)
{
	if (USkeletalMeshComponent* EquipMesh = *PlayerMeshes.Find(Slot))
	{
		if (USkeletalMesh* BodyMesh = *NakedMeshes.Find(Slot))
		{
			EquipMesh->SetSkeletalMeshAsset(BodyMesh);
			//Set Material for new equip item

			for (int32 i = 0; i < BodyMesh->GetMaterials().Num(); ++i)
			{
				/*if (BodyMesh->GetMaterials().IsValidIndex(i))
				{
					EquipMesh->SetMaterial(i, BodyMesh->Materials[i].MaterialInterface);
				}*/
			}
		}
		else
		{
			//Logic for meshes without base mesh; ie backpack
			EquipMesh->SetSkeletalMeshAsset(nullptr);
		}
	}
}

void AAOPlayerBase::EquipWeapon(class UAOWeaponItem* WeaponItem)
{
	if (WeaponItem && WeaponItem->WeaponClass)
	{
		if (EquippedWeapon)
		{
			UnequipWeapon();
		}
		//Spawn in Weapon
		FActorSpawnParameters SpawnParams;
		SpawnParams.bNoFail = true;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		SpawnParams.Owner = SpawnParams.Instigator = this;

		if (AAOWeapon* Weapon = GetWorld()->SpawnActor<AAOWeapon>(WeaponItem->WeaponClass, SpawnParams))
		{
			Weapon->Item = WeaponItem;

			EquippedWeapon = Weapon;
			OnWeaponEquipped();
			//Attach Weapon to Pawn
			Weapon->OnEquip();
		}
	}
}

void AAOPlayerBase::UnequipWeapon()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->OnUnEquip();
		EquippedWeapon->Destroy();
		EquippedWeapon = nullptr;
		OnWeaponEquipped();
	}
}

class USkeletalMeshComponent* AAOPlayerBase::GetSlotSkeletalMeshComponent(const EEquipSlot Slot)
{
	if (PlayerMeshes.Contains(Slot))
	{
		return *PlayerMeshes.Find(Slot);
	}
	return nullptr;
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

void AAOPlayerBase::Attack()
{
	/*if (CanAttack())
	{
		ActionState = EActionState::EAS_Attacking;
	}*/
}

void AAOPlayerBase::UseThrowable()
{
	if (CanUseThrowable())
	{
		if (UAOThrowItem* Throwable = GetThrowable())
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
	}
}


