// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Character/AOEnemy.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Abilities/AOAbilitySystemComponent.h"
#include "Attributes/AOAttributeSet.h"
#include "Components/WidgetComponent.h"
#include "UI/AOUserWidget.h"
#include <Abilities/BFLAbilitySystem.h>
#include "AOGameplayTags.h"
#include "AI/AOAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"

AAOEnemy::AAOEnemy()
{
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	AbilitySystemComponent = CreateDefaultSubobject<UAOAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	AttributeSet = CreateDefaultSubobject<UAOAttributeSet>("AttributeSet");

	Health = CreateDefaultSubobject<UWidgetComponent>("Health Bar");
	Health->SetupAttachment(GetRootComponent());

	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetGenerateOverlapEvents(true);

	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	Weapon->SetupAttachment(GetMesh(), FName("weapon"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AAOEnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	AIC = Cast<AAOAIController>(NewController);
	AIC->GetBlackboardComponent()->InitializeBlackboard(*BTree->BlackboardAsset);
	AIC->RunBehaviorTree(BTree);
	AIC->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), false);
	AIC->GetBlackboardComponent()->SetValueAsBool(FName("RangedAttacker"), CharacterClass != ECharacterClass::Warrior);
}

int32 AAOEnemy::GetPlayerLevel()
{
	return Level;
}



void AAOEnemy::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	InitAbilityActorInfo();
	UBFLAbilitySystem::GiveStartupAbilities(this, AbilitySystemComponent, CharacterClass);
	
	//Set widget controller for enemy health bar
	if (UAOUserWidget* EnemyUI = Cast<UAOUserWidget>(Health->GetUserWidgetObject()))
	{
		EnemyUI->SetWidgetController(this);
	}
	//Set up binding value changes (lambdas) for Progress bar
	if (const UAOAttributeSet* AS = Cast<UAOAttributeSet>(AttributeSet))
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

		AbilitySystemComponent->RegisterGameplayTagEvent(FAOGameplayTags::Get().Effects_HitReact, EGameplayTagEventType::NewOrRemoved).AddUObject(this,&AAOEnemy::HitReactTagChanged);
		//Set Initial values for attributes
		OnHealthChange.Broadcast(AS->GetHealth());
		OnMaxHealthChange.Broadcast(AS->GetMaxHealth());
	}
}

void AAOEnemy::HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bHitReacting = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed = bHitReacting ? 0.f : BaseWalkSpeed;
	AIC->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), bHitReacting);
}

AActor* AAOEnemy::GetCombatTarget_Implementation() const
{
	return CombatTarget;
}

void AAOEnemy::SetCombatTarget_Implementation(AActor* InCombatTarget)
{
	CombatTarget = InCombatTarget;
}

void AAOEnemy::Die()
{
	SetLifeSpan(LifeSpan);
	
	Weapon->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
	Weapon->SetSimulatePhysics(true);
	Weapon->SetEnableGravity(true);
	Weapon->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	
	Super::Die();

	StartDissolve();
}

void AAOEnemy::InitAbilityActorInfo()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	Cast<UAOAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();

	InitializeAttributes();
}

void AAOEnemy::InitializeAttributes() const
{
	UBFLAbilitySystem::InitializeDefaultAttributes(this, CharacterClass, Level, AbilitySystemComponent);
}

FVector AAOEnemy::GetCombatSocketLocation_Implementation(const FGameplayTag& MontageTag)
{

	const FAOGameplayTags& GameplayTags = FAOGameplayTags::Get();
	if (MontageTag.MatchesTagExact(GameplayTags.Montage_Attack_Weapon) && IsValid(Weapon))
	{
		return Weapon->GetSocketLocation(WeaponSocket);
	}
	if (MontageTag.MatchesTagExact(GameplayTags.Montage_Attack_LHand))
	{
		return GetMesh()->GetSocketLocation(LHand);
	}
	if (MontageTag.MatchesTagExact(GameplayTags.Montage_Attack_RHand))
	{
		return GetMesh()->GetSocketLocation(RHand);
	}
	return FVector();
}