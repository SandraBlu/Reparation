// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/REnemy.h"
#include "RGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "AI/RAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "Components/Combat/REnemyMeleeSweepComponent.h"
#include "Framework/RAbilitySystemLibrary.h"
#include "Framework/RGameMode.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/RAbilitySystemComponent.h"
#include "GAS/RAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "Reparation/Reparation.h"
#include "UI/GAS/RUserWidget.h"
#include "GAS/Data/RElementalResistanceProfile.h"
#include "GAS/Ability/RDamageAbility.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"
#include "HAL/IConsoleManager.h"

static bool IsParryPresentationDebugEnabled()
{
	const auto* Variable=IConsoleManager::Get().FindConsoleVariable(TEXT("r.Combat.ParryDebug"));
	return Variable && Variable->GetInt()!=0;
}

AREnemy::AREnemy(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = CreateDefaultSubobject<URAbilitySystemComponent>("ASC");
	AttributeSet = CreateDefaultSubobject<URAttributeSet>("Attributes");
	MeleeSweep = CreateDefaultSubobject<UREnemyMeleeSweepComponent>(TEXT("MeleeSweep"));

	HealthBar = CreateDefaultSubobject<UWidgetComponent>("Health");
	HealthBar->SetupAttachment(GetRootComponent());
	
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	WeaponRH = CreateDefaultSubobject<USkeletalMeshComponent>("CombatRH");
	WeaponRH->SetupAttachment(GetMesh(), FName("weaponRH"));
	WeaponRH->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	WeaponLH = CreateDefaultSubobject<USkeletalMeshComponent>("CombatLH");
	WeaponLH->SetupAttachment(GetMesh(), FName("weaponLH"));
	WeaponLH->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f,180.f,0.f);
	GetCharacterMovement()->BrakingDecelerationWalking = 1000.f;
	
	CombatComp = CreateDefaultSubobject<UPawnCombatComponent>(TEXT("EnemyCombatComp"));
}

void AREnemy::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	InitAbilityActorInfo();
	if (HasAuthority())
	{
		URAbilitySystemLibrary::GiveStartupAbilities(this, AbilitySystemComponent, CharacterClass);
	}
	
	if (URUserWidget* EnemyHealthUI = Cast<URUserWidget>(HealthBar->GetUserWidgetObject()))
	{
		EnemyHealthUI->SetWidgetController(this);
	}

	if (const URAttributeSet* AS = Cast<URAttributeSet>(AttributeSet))
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AS->GetHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnHealthChange.Broadcast(Data.NewValue);
			}
		);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AS->GetMaxHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxHealthChange.Broadcast(Data.NewValue);
			}
		);
		//Call Hit React/Stunned function
		AbilitySystemComponent->RegisterGameplayTagEvent(FRGameplayTags::Get().ability_HitReact, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AREnemy::HitReactTagChanged);
		//Broadcast Initial Values
		OnHealthChange.Broadcast(AS->GetHealth());
		OnMaxHealthChange.Broadcast(AS->GetMaxHealth());
	}
}

void AREnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	AIC = Cast<ARAIController>(NewController);
	AIC->GetBlackboardComponent()->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
	AIC->RunBehaviorTree(BehaviorTree);
	AIC->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), false);
	AIC->GetBlackboardComponent()->SetValueAsBool(FName("Stunned"), false);
	AIC->GetBlackboardComponent()->SetValueAsBool(FName("RangedAttacker"), CharacterClass != ECharacterClass::Warrior);
}

int32 AREnemy::GetPlayerLevel_Implementation()
{
	return FMath::Max(1, Level);
}

FVector AREnemy::GetCombatSocketLocation_Implementation(const FGameplayTag& CombatSocketTag)
{
	const FRGameplayTags& GameplayTags = FRGameplayTags::Get();
	if (CombatSocketTag.MatchesTagExact(GameplayTags.combatSocket_weapon))
	{
		if (USkeletalMeshComponent* DamageMesh = GetMeleeDamageMesh()) return DamageMesh->GetSocketLocation(WeaponDamageSocket);
	}
	if (CombatSocketTag.MatchesTagExact(GameplayTags.combatSocket_handL))
	{
		return GetMesh()->GetSocketLocation(HandLSocket);
	}
	if (CombatSocketTag.MatchesTagExact(GameplayTags.combatSocket_handR))
	{
		return GetMesh()->GetSocketLocation(HandRSocket);
	}
	return FVector();
}

USkeletalMeshComponent* AREnemy::GetMeleeDamageMesh() const
{
	if (IsValid(WeaponRH) && WeaponRH->GetSkeletalMeshAsset() && WeaponRH->DoesSocketExist(WeaponDamageSocket)) return WeaponRH;
	if (IsValid(WeaponLH) && WeaponLH->GetSkeletalMeshAsset() && WeaponLH->DoesSocketExist(WeaponDamageSocket)) return WeaponLH;
	return nullptr;
}

void AREnemy::GetMeleeRangeForTarget(const AActor* Target, float& Minimum, float& Preferred, float& Maximum) const
{
	const ACharacter* TargetCharacter = Cast<ACharacter>(Target);
	const bool bCrouchedTarget = TargetCharacter && TargetCharacter->bIsCrouched;
	Minimum = FMath::Max(1.f, bCrouchedTarget ? MinMeleeDistance : StandingMinMeleeDistance);
	Maximum = FMath::Max(Minimum, bCrouchedTarget ? MaxMeleeDistance : StandingMaxMeleeDistance);
	Preferred = FMath::Clamp(bCrouchedTarget ? PreferredMeleeDistance : StandingPreferredMeleeDistance, Minimum, Maximum);
}

void AREnemy::Die(const FVector& DeathImpulse)
{
	SetLifeSpan(LifeSpan);
	MulticastHandleDeath(DeathImpulse);
	if (AIC) AIC->GetBlackboardComponent()->SetValueAsBool(FName("IsDead"), true);
	DissolveMesh();
	
}

void AREnemy::MulticastHandleDeath_Implementation(const FVector& DeathImpulse)
{
	WeaponRH->SetSimulatePhysics(true);
	WeaponRH->SetEnableGravity(true);
	WeaponRH->SetCollisionEnabled(ECollisionEnabled::Type::PhysicsOnly);
	WeaponRH->AddImpulse(DeathImpulse);

	WeaponLH->SetSimulatePhysics(true);
	WeaponLH->SetEnableGravity(true);
	WeaponLH->SetCollisionEnabled(ECollisionEnabled::Type::PhysicsOnly);
	WeaponLH->AddImpulse(DeathImpulse);

	HandleDroppedItems();
	
	UGameplayStatics::PlaySoundAtLocation(this, DeathCry, GetActorLocation(), GetActorRotation());
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::Type::PhysicsOnly);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	GetMesh()->AddImpulse(DeathImpulse, NAME_None, true);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	bDead = true;
	bNeedsKnockdownRecovery = false;
	bRecovering = false;
	++RecoveryGeneration;
	SetToStunned(false);
	StaggerTagChanged(FRGameplayTags::Get().Debuff_Stagger,0);
	OnDeath.Broadcast(this);
}

void AREnemy::SetCombatTarget_Implementation(AActor* InCombatTarget)
{
	CombatTarget = InCombatTarget;
}

AActor* AREnemy::GetCombatTarget_Implementation() const
{
	return CombatTarget;
}

void AREnemy::HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bHitReacting = NewCount > 0;
	RefreshControlMovement();
	if (AIC && AIC->GetBlackboardComponent())
	{
		AIC->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), bHitReacting);
	}
}

void AREnemy::StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount) { SetToStunned(NewCount > 0); }
void AREnemy::FreezeTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bFrozen = NewCount > 0 && (!ElementalResistanceProfile || !ElementalResistanceProfile->bImmuneToControl);
	RefreshControlMovement();
}

void AREnemy::SetToStunned(bool bIsStunned)
{
	const bool bWasStunned = bStunned;
	bStunned = bIsStunned && !bDead && (!ElementalResistanceProfile || !ElementalResistanceProfile->bImmuneToControl);
	if (!bStunned && bWasStunned && !bStaggered && !bDead && bNeedsKnockdownRecovery) bRecovering = true;
	RefreshControlMovement();
	if (bStunned && !bWasStunned) { CancelControlAttacks(); PlayControlMontage(GetStunnedMontage_Implementation(),true); }
	else if (!bStunned && bWasStunned)
	{
		if (auto* Anim = GetMesh()->GetAnimInstance(); Anim && GetStunnedMontage_Implementation()) Anim->Montage_Stop(.15f,GetStunnedMontage_Implementation());
		if (!bDead && !bStaggered)
		{
			if (bNeedsKnockdownRecovery) StartKnockdownRecovery();
			else PlayControlMontage(StunRecoveryMontage,false);
		}
	}
}

void AREnemy::RefreshControlMovement()
{
	GetCharacterMovement()->MaxWalkSpeed = bDead || bStunned || bStaggered || bRecovering || bHitReacting ? 0.f : BaseWalkSpeed * (bFrozen ? .65f : 1.f);
	if (AIC && AIC->GetBlackboardComponent()) AIC->GetBlackboardComponent()->SetValueAsBool(TEXT("Stunned"),bStunned || bStaggered || bRecovering);
}
void AREnemy::CancelControlAttacks()
{
	TArray<FGameplayAbilitySpecHandle> Handles;
	for (const auto& Spec : AbilitySystemComponent->GetActivatableAbilities())
		if (Spec.IsActive() && Spec.Ability && Spec.Ability->IsA<URDamageAbility>()) Handles.Add(Spec.Handle);
	for (const auto Handle : Handles) AbilitySystemComponent->CancelAbilityHandle(Handle);
}
void AREnemy::PlayControlMontage(UAnimMontage* Montage,bool bLoop)
{
	if (bApplyingParryReaction && IsParryPresentationDebugEnabled())
		UE_LOG(LogTemp,Display,TEXT("ParryDebug: %s requests enemy reaction %s (anim=%s dead=%d)."),
			*GetName(),*GetNameSafe(Montage),*GetNameSafe(GetMesh()->GetAnimInstance()),bDead);
	if (!Montage || bDead) return;
	if (auto* Anim = GetMesh()->GetAnimInstance())
	{
		const float PlayedLength=Anim->Montage_Play(Montage);
		if (bApplyingParryReaction && IsParryPresentationDebugEnabled())
			UE_LOG(LogTemp,Display,TEXT("ParryDebug: %s enemy montage playback returned %.3fs."),*GetName(),PlayedLength);
		if (PlayedLength>0.f && bApplyingParryReaction)
			Anim->OnMontageEnded.AddUniqueDynamic(this,&AREnemy::ParriedPresentationEnded);
		if (PlayedLength>0.f && bLoop && Montage->CompositeSections.Num()>0)
			Anim->Montage_SetNextSection(Montage->GetSectionName(Montage->CompositeSections.Num()-1),Montage->GetSectionName(0),Montage);
	}
}
void AREnemy::ParriedPresentationEnded(UAnimMontage* Montage,bool bInterrupted)
{
	if (Montage!=ParriedMontage) return;
	if (IsParryPresentationDebugEnabled())
		UE_LOG(LogTemp,Display,TEXT("ParryDebug: %s enemy reaction %s ended (interrupted=%d, staggered=%d, stunned=%d, dead=%d)."),
			*GetName(),*GetNameSafe(Montage),bInterrupted,bStaggered,bStunned,bDead);
}
void AREnemy::StaggerTagChanged(const FGameplayTag CallbackTag,int32 NewCount)
{
	const bool bWas = bStaggered;
	bStaggered = NewCount>0 && !bDead && (!ElementalResistanceProfile || !ElementalResistanceProfile->bImmuneToControl);
	if (!bStaggered && bWas && !bStunned && !bDead && bNeedsKnockdownRecovery) bRecovering = true;
	RefreshControlMovement();
	if (bStaggered && !bWas)
	{
		CancelControlAttacks();
		if (!bStunned)
		{
			ActiveStaggerMontage = bApplyingParryReaction ? ParriedMontage : StaggerMontage;
			PlayControlMontage(ActiveStaggerMontage,false);
		}
	}
	else if (!bStaggered && bWas && !bStunned)
	{
		if (bNeedsKnockdownRecovery && !bDead) StartKnockdownRecovery();
		else if (auto* Anim = GetMesh()->GetAnimInstance(); Anim && ActiveStaggerMontage) Anim->Montage_Stop(.1f,ActiveStaggerMontage);
	}
}
bool AREnemy::ApplyParryStagger(float Seconds, AActor* Source)
{
	TGuardValue<bool> ParryReaction(bApplyingParryReaction,true);
	const bool bWasStaggered = bStaggered;
	if (!ApplyControlEffect(true,Seconds,Source))
	{
		if (IsParryPresentationDebugEnabled())
			UE_LOG(LogTemp,Display,TEXT("ParryDebug: %s rejected enemy parry control (duration=%.3f, dead=%d, authority=%d)."),*GetName(),Seconds,bDead,HasAuthority());
		return false;
	}
	if (bStunned && IsParryPresentationDebugEnabled())
		UE_LOG(LogTemp,Display,TEXT("ParryDebug: %s defers enemy parry presentation because full stun is active."),*GetName());
	if (bWasStaggered && !bStunned && !bNeedsKnockdownRecovery)
	{
		if (auto* Anim = GetMesh()->GetAnimInstance(); Anim && ActiveStaggerMontage)
			Anim->Montage_Stop(.1f,ActiveStaggerMontage);
		ActiveStaggerMontage = ParriedMontage;
		PlayControlMontage(ActiveStaggerMontage,false);
	}
	return true;
}
bool AREnemy::ApplyKnockdown(float Seconds, AActor* Source)
{
	const bool bWasStaggered = bStaggered;
	if (!ApplyControlEffect(true,Seconds,Source)) return false;
	bNeedsKnockdownRecovery = true;
	if (bWasStaggered && !bStunned)
	{
		ActiveStaggerMontage = StaggerMontage;
		PlayControlMontage(ActiveStaggerMontage,false);
	}
	// Hold the final blast-back pose until the ability's timed stagger expires.
	if (auto* Anim = GetMesh()->GetAnimInstance())
		if (auto* Instance = Anim->GetActiveInstanceForMontage(StaggerMontage)) Instance->bEnableAutoBlendOut = false;
	return true;
}
void AREnemy::StartKnockdownRecovery()
{
	if (bDead || bStunned || bStaggered || !bNeedsKnockdownRecovery) return;
	const uint32 Generation = ++RecoveryGeneration;
	bRecovering = true;
	RefreshControlMovement();
	CancelControlAttacks();
	auto* Anim = GetMesh()->GetAnimInstance();
	if (!KnockdownRecoveryMontage || !Anim || Anim->Montage_Play(KnockdownRecoveryMontage) <= 0.f)
	{
		if (Anim && StaggerMontage) Anim->Montage_Stop(.1f,StaggerMontage);
		FinishKnockdownRecovery(KnockdownRecoveryMontage,true,Generation);
		return;
	}
	FOnMontageEnded Ended;
	Ended.BindWeakLambda(this,[this,Generation](UAnimMontage* Montage,bool bInterrupted)
	{
		FinishKnockdownRecovery(Montage,bInterrupted,Generation);
	});
	Anim->Montage_SetEndDelegate(Ended,KnockdownRecoveryMontage);
}
void AREnemy::FinishKnockdownRecovery(UAnimMontage* Montage,bool bInterrupted,uint32 Generation)
{
	if (Generation != RecoveryGeneration) return;
	bRecovering = false;
	// A new control effect interrupts get-up and will retry it on its own expiry.
	if (!bStunned && !bStaggered) bNeedsKnockdownRecovery = false;
	RefreshControlMovement();
}
void AREnemy::BindControlTags()
{
	const auto& ControlTags=FRGameplayTags::Get();
	AbilitySystemComponent->RegisterGameplayTagEvent(ControlTags.Debuff_Stun).RemoveAll(this);
	AbilitySystemComponent->RegisterGameplayTagEvent(ControlTags.Debuff_Stun).AddUObject(this,&AREnemy::StunTagChanged);
	AbilitySystemComponent->RegisterGameplayTagEvent(ControlTags.Debuff_Stagger).RemoveAll(this);
	AbilitySystemComponent->RegisterGameplayTagEvent(ControlTags.Debuff_Stagger).AddUObject(this,&AREnemy::StaggerTagChanged);
	AbilitySystemComponent->RegisterGameplayTagEvent(ControlTags.Debuff_Freeze).RemoveAll(this);
	AbilitySystemComponent->RegisterGameplayTagEvent(ControlTags.Debuff_Freeze).AddUObject(this,&AREnemy::FreezeTagChanged);
	StunTagChanged(ControlTags.Debuff_Stun,AbilitySystemComponent->GetTagCount(ControlTags.Debuff_Stun));
	StaggerTagChanged(ControlTags.Debuff_Stagger,AbilitySystemComponent->GetTagCount(ControlTags.Debuff_Stagger));
	FreezeTagChanged(ControlTags.Debuff_Freeze,AbilitySystemComponent->GetTagCount(ControlTags.Debuff_Freeze));
}
bool AREnemy::ApplyControlEffect(bool bStagger,float Seconds,AActor* Source)
{
	if (!HasAuthority() || bDead || !FMath::IsFinite(Seconds) || Seconds<=0 || (ElementalResistanceProfile && ElementalResistanceProfile->bImmuneToControl)) return false;
	BindControlTags();
	auto* Effect=NewObject<UGameplayEffect>();
	Effect->DurationPolicy=EGameplayEffectDurationType::HasDuration;
	Effect->DurationMagnitude=FScalableFloat(FMath::Clamp(Seconds,.01f,30.f));
	FInheritedTagContainer GrantedTags; GrantedTags.Added.AddTag(bStagger ? FRGameplayTags::Get().Debuff_Stagger : FRGameplayTags::Get().Debuff_Stun);
	Effect->FindOrAddComponent<UTargetTagsGameplayEffectComponent>().SetAndApplyTargetTagChanges(GrantedTags);
	auto Context=AbilitySystemComponent->MakeEffectContext(); Context.AddSourceObject(Source);
	return AbilitySystemComponent->ApplyGameplayEffectToSelf(Effect,1.f,Context).IsValid();
}
void AREnemy::ApplyLegacyStun(bool Stunned,float StunDuration)
{
	if (Stunned) ApplyControlEffect(false,StunDuration);
	else SetToEndStun(); // A legacy recovery request cannot clear another active GAS effect.
}

void AREnemy::SetToEndStun()
{
	SetToStunned(AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(FRGameplayTags::Get().Debuff_Stun));
}

void AREnemy::InitAbilityActorInfo()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	BindControlTags();
	Cast<URAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();
	if (HasAuthority())
	{
		InitializeAttributes();
	}
	OnASCRegistered.Broadcast(AbilitySystemComponent);
}

void AREnemy::InitializeAttributes() const
{
	URAbilitySystemLibrary::InitializeDefaultAttributes(this, CharacterClass, Level, AbilitySystemComponent);
}
