// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RCompanion.h"

#include "AI/AIC_Companion.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "GAS/RAbilitySystemComponent.h"
#include "GAS/RAttributeSet.h"
#include "GAS/Ability/RCompanionAttackAbility.h"
#include "AI/REnemy.h"
#include "Framework/RAbilitySystemLibrary.h"


ARCompanion::ARCompanion(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AIControllerClass = AAIC_Companion::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	AbilitySystemComponent = CreateDefaultSubobject<URAbilitySystemComponent>(TEXT("CompanionASC"));
	AttributeSet = CreateDefaultSubobject<URAttributeSet>(TEXT("CompanionAttributes"));
	CombatComp = CreateDefaultSubobject<UPawnCombatComponent>(TEXT("CompanionCombat"));
	Tags.AddUnique(TEXT("player"));
	RightWeaponSlot = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RightWeaponSlot"));
	LeftWeaponSlot = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LeftWeaponSlot"));
	RightWeaponSlot->SetupAttachment(GetMesh(), RightWeaponSocket);
	LeftWeaponSlot->SetupAttachment(GetMesh(), LeftWeaponSocket);
	for (USkeletalMeshComponent* Slot : {RightWeaponSlot.Get(), LeftWeaponSlot.Get()})
	{
		Slot->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Slot->SetGenerateOverlapEvents(false);
		Slot->bReceivesDecals = false;
	}
	SelectionMarker = CreateDefaultSubobject<UTextRenderComponent>(TEXT("CompanionSelectionMarker"));
	SelectionMarker->SetupAttachment(GetRootComponent());
	SelectionMarker->SetRelativeLocation(FVector(0.f, 0.f, 130.f));
	SelectionMarker->SetText(NSLOCTEXT("CompanionUI", "Selected", "Selected"));
	SelectionMarker->SetHorizontalAlignment(EHTA_Center);
	SelectionMarker->SetWorldSize(22.f);
	SelectionMarker->SetTextRenderColor(FColor(64, 204, 255));
	SelectionMarker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SelectionMarker->SetVisibility(false);
}

void ARCompanion::BeginPlay()
{
	Super::BeginPlay();
	RefreshWeaponAttachments();
	InitAbilityActorInfo();
	const FText Issue = GetCombatReadinessIssue();
	if (!Issue.IsEmpty()) UE_LOG(LogTemp, Warning, TEXT("Companion %s combat unavailable: %s"), *GetName(), *Issue.ToString());
}

void ARCompanion::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RefreshWeaponAttachments();
}

void ARCompanion::RefreshWeaponAttachments()
{
	auto AttachSlot = [this](USkeletalMeshComponent* Slot, FName Socket)
	{
		if (!Slot) return;
		const bool bHasMesh = Slot->GetSkeletalMeshAsset() != nullptr;
		const bool bValidSocket = GetMesh() && GetMesh()->DoesSocketExist(Socket);
		if (bValidSocket)
			Slot->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, Socket);
		Slot->SetVisibility(bHasMesh && bValidSocket);
		if (bHasMesh && !bValidSocket)
			UE_LOG(LogTemp, Warning, TEXT("Companion %s: weapon socket %s is missing on the character mesh. The weapon slot is unavailable."), *GetName(), *Socket.ToString());
	};
	AttachSlot(RightWeaponSlot, RightWeaponSocket);
	AttachSlot(LeftWeaponSlot, LeftWeaponSocket);
}

bool ARCompanion::HasEquippedWeapon() const
{
	auto IsEquipped = [this](const USkeletalMeshComponent* Slot, FName Socket)
	{
		return Slot && Slot->GetSkeletalMeshAsset() && GetMesh() && GetMesh()->DoesSocketExist(Socket) &&
			Slot->GetAttachParent() == GetMesh() && Slot->GetAttachSocketName() == Socket && Slot->IsVisible() && !Slot->bHiddenInGame;
	};
	return IsEquipped(RightWeaponSlot, RightWeaponSocket) || IsEquipped(LeftWeaponSlot, LeftWeaponSocket);
}

bool ARCompanion::HasAssignedWeapon() const
{
	auto IsAssigned = [this](const USkeletalMeshComponent* Slot, FName Socket)
	{
		return Slot && Slot->GetSkeletalMeshAsset() && GetMesh() && GetMesh()->DoesSocketExist(Socket);
	};
	return IsAssigned(RightWeaponSlot, RightWeaponSocket) || IsAssigned(LeftWeaponSlot, LeftWeaponSocket);
}

bool ARCompanion::EquipAssignedWeapons()
{
	RefreshWeaponAttachments();
	for (USkeletalMeshComponent* Slot : {RightWeaponSlot.Get(), LeftWeaponSlot.Get()})
		if (Slot && Slot->GetSkeletalMeshAsset()) Slot->SetHiddenInGame(false);
	return HasEquippedWeapon();
}

void ARCompanion::InitAbilityActorInfo()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	// Recompiled Blueprint instances can retain an additional attribute-set
	// subobject. GAS resolves attributes by class, so use our owned set and keep
	// unrelated attribute-set classes intact.
	const TArray<UAttributeSet*> ExistingSets = AbilitySystemComponent->GetSpawnedAttributes();
	for (UAttributeSet* Existing : ExistingSets)
	{
		if (Existing && Existing != AttributeSet && Existing->IsA<URAttributeSet>())
			AbilitySystemComponent->RemoveSpawnedAttribute(Existing);
	}
	AbilitySystemComponent->AddAttributeSetSubobject(AttributeSet);
	CastChecked<URAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();
	if (HasAuthority())
	{
		if (PrimaryAttributes && SecondaryAttributes && ResistanceAttributes && VitalAttributes)
		{
			InitializeAttributes();
			bAttributesInitialized = CastChecked<URAttributeSet>(AttributeSet)->GetMaxHealth() > 0.f;
			GrantAbilities();
			if (AttackAbility && !AbilitySystemComponent->FindAbilitySpecFromClass(AttackAbility))
				AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AttackAbility, CompanionLevel));
			if (SignatureAbility && !AbilitySystemComponent->FindAbilitySpecFromClass(SignatureAbility))
				AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(SignatureAbility, CompanionLevel));
		}
		else UE_LOG(LogTemp, Warning, TEXT("Companion %s needs Primary, Secondary, Resistance and Vital attribute effects before combat is available."), *GetName());
	}
	OnASCRegistered.Broadcast(AbilitySystemComponent);
}

void ARCompanion::InitializeAttributes() const
{
	ApplyEffectToSelf(PrimaryAttributes, CompanionLevel);
	ApplyEffectToSelf(SecondaryAttributes, CompanionLevel);
	ApplyEffectToSelf(ResistanceAttributes, CompanionLevel);
	ApplyEffectToSelf(VitalAttributes, CompanionLevel);
}

int32 ARCompanion::GetPlayerLevel_Implementation() { return CompanionLevel; }
void ARCompanion::SetCombatTarget_Implementation(AActor* InCombatTarget) { CombatTarget = InCombatTarget; }
AActor* ARCompanion::GetCombatTarget_Implementation() const { return CombatTarget; }

bool ARCompanion::IsCombatReady() const
{
	return GetCombatReadinessIssue().IsEmpty();
}

FText ARCompanion::GetCombatReadinessIssue() const
{
	if (!HasAuthority()) return FText::FromString(TEXT("Companion combat is unavailable on this client."));
	if (bDead) return FText::FromString(TEXT("This companion is dead."));
	if (!PrimaryAttributes) return FText::FromString(TEXT("Companion is missing its Primary Attributes effect."));
	if (!SecondaryAttributes) return FText::FromString(TEXT("Companion is missing its Secondary Attributes effect."));
	if (!ResistanceAttributes) return FText::FromString(TEXT("Companion is missing its Resistance Attributes effect."));
	if (!VitalAttributes) return FText::FromString(TEXT("Companion is missing its Vital Attributes effect."));
	if (!bAttributesInitialized) return FText::FromString(TEXT("Companion health was not initialized. Restart PIE after changing attribute effects."));
	if (bRequiresEquippedWeapon && !HasAssignedWeapon())
	{
		if (RightWeaponSlot && RightWeaponSlot->GetSkeletalMeshAsset()) return FText::Format(FText::FromString(TEXT("Assigned right weapon needs socket {0} on the character mesh.")), FText::FromName(RightWeaponSocket));
		if (LeftWeaponSlot && LeftWeaponSlot->GetSkeletalMeshAsset()) return FText::Format(FText::FromString(TEXT("Assigned left weapon needs socket {0} on the character mesh.")), FText::FromName(LeftWeaponSocket));
		return FText::FromString(TEXT("No weapon mesh assigned to this companion's RightWeaponSlot or LeftWeaponSlot."));
	}
	if (!AttackAbility) return FText::FromString(TEXT("No companion Attack Ability assigned."));
	if (!AbilitySystemComponent || !AbilitySystemComponent->FindAbilitySpecFromClass(AttackAbility)) return FText::FromString(TEXT("Assigned Attack Ability was not granted. Restart PIE after changing the ability."));
	const URCompanionAttackAbility* Defaults = AttackAbility->GetDefaultObject<URCompanionAttackAbility>();
	return Defaults ? Defaults->GetConfigurationIssue(this) : FText::FromString(TEXT("Attack Ability defaults are unavailable."));
}

bool ARCompanion::CanAttackTarget(AActor* Target) const
{
	const ARCharacterBase* TargetCharacter = Cast<ARCharacterBase>(Target);
	return IsValid(Target) && Target != this && (Target->IsA<AREnemy>() || Target->ActorHasTag(TEXT("enemy")))
		&& !Target->ActorHasTag(TEXT("player")) && Target->Implements<URCombatInterface>()
		&& (!TargetCharacter || !TargetCharacter->IsDead_Implementation())
		&& !IRCombatInterface::Execute_IsDead(Target) && Target->FindComponentByClass<UAbilitySystemComponent>();
}

bool ARCompanion::IsAttackActive() const
{
	const FGameplayAbilitySpec* Spec = AttackAbility ? AbilitySystemComponent->FindAbilitySpecFromClass(AttackAbility) : nullptr;
	return Spec && Spec->IsActive();
}

bool ARCompanion::TryAttack()
{
	if (IsCombatReady() && CanAttackTarget(CombatTarget) && !IsAttackActive() && SignatureAbility && GetWorld()->GetTimeSeconds() >= NextSignatureTime)
		if (AbilitySystemComponent->TryActivateAbilityByClass(SignatureAbility, false)) NextSignatureTime = GetWorld()->GetTimeSeconds() + FMath::Max(1.f, SignatureCooldown);
	return IsCombatReady() && CanAttackTarget(CombatTarget) && AbilitySystemComponent->TryActivateAbilityByClass(AttackAbility, false) && IsAttackActive();
}

void ARCompanion::CancelAttack()
{
	if (FGameplayAbilitySpec* Spec = AttackAbility ? AbilitySystemComponent->FindAbilitySpecFromClass(AttackAbility) : nullptr)
		AbilitySystemComponent->CancelAbilityHandle(Spec->Handle);
	CombatTarget = nullptr;
}

void ARCompanion::Die(const FVector& DeathImpulse)
{
	if (bDead) return;
	bDead = true;
	if (AAIC_Companion* AI = Cast<AAIC_Companion>(GetController())) AI->Stay();
	AbilitySystemComponent->CancelAllAbilities();
	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SelectionMarker->SetVisibility(false);
	OnDeath.Broadcast(this);
}
