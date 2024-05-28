// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RNPC.h"
#include "Components/WidgetComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystem/RAbilitySystemComponent.h"
#include "AbilitySystem//RAttributeSet.h"
#include "Components/WidgetComponent.h"
#include "RGameplayTags.h"
#include "Kismet/GameplayStatics.h"

ARNPC::ARNPC()
{
	AbilitySystemComponent = CreateDefaultSubobject<URAbilitySystemComponent>("ASC");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<URAttributeSet>("Attributes");
	
	GetCharacterMovement()->bUseControllerDesiredRotation = true;

	Health = CreateDefaultSubobject<UWidgetComponent>("Health");
	Health->SetupAttachment(GetRootComponent());

	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>("PawnSensingComp");

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	//GetMesh()->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Combat");
	Weapon->SetupAttachment(GetMesh(), FName("weapon"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ARNPC::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	/*AIC = Cast<AAOAIController>(NewController);
	AIC->GetBlackboardComponent()->InitializeBlackboard(*BTree->BlackboardAsset);
	AIC->RunBehaviorTree(BTree);
	AIC->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), false);
	AIC->GetBlackboardComponent()->SetValueAsBool(FName("RangedAttacker"), CharacterClass != ECharacterClass::Warrior);*/
}

void ARNPC::InitAbilityActorInfo()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	Cast<URAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();

	//InitializeAttributes();
}

void ARNPC::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	InitAbilityActorInfo();


	////Set widget controller for enemy health bar
	//if (URUserWidget* EnemyHealthUI = Cast<URUserWidget>(Health->GetUserWidgetObject()))
	//{
	//	EnemyHealthUI->SetWidgetController(this);
	//}
	////Set up binding value changes (lambdas) for Progress bar
	//if (const URAttributeSet* AS = Cast<URAttributeSet>(AttributeSet))
	//{
	//	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AS->GetHealthAttribute()).AddLambda(
	//		[this](const FOnAttributeChangeData& Data)
	//		{
	//			OnHealthChange.Broadcast(Data.NewValue);
	//		}
	//	);

	//	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AS->GetMaxHealthAttribute()).AddLambda(
	//		[this](const FOnAttributeChangeData& Data)
	//		{
	//			OnMaxHealthChange.Broadcast(Data.NewValue);
	//		}
	//	);
	//	//Hit react Event Tag to GE_HitReact
	//	AbilitySystemComponent->RegisterGameplayTagEvent(FRGameplayTags::Get().ability_HitReact, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ARNPC::HitReactTagChanged);


	//	//Set Initial values for attributes
	//	OnHealthChange.Broadcast(AS->GetHealth());
	//	OnMaxHealthChange.Broadcast(AS->GetMaxHealth());
	//}
}
void ARNPC::Die()
{
	SetLifeSpan(LifeSpan);
	//if (AIC) AIC->GetBlackboardComponent()->SetValueAsBool(FName("IsDead"), true);
	Weapon->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
	Weapon->SetSimulatePhysics(true);
	Weapon->SetEnableGravity(true);
	Weapon->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);

	Super::Die();

	DissolveMesh();
}

void ARNPC::DirectionalHitReact(const FVector& ImpactPoint)
{
	const FVector Forward = GetActorForwardVector();
	//Lower Impact Point to Enemy's .Z location
	const FVector ImpactLowered(ImpactPoint.X, ImpactPoint.Y, GetActorLocation().Z);
	const FVector HitDist = (ImpactLowered - GetActorLocation()).GetSafeNormal();


	//Find Angle
	//(Forward(1) * HitDist(1)) * cos(theta) = DOt Product
	const double CosTheta = FVector::DotProduct(Forward, HitDist);
	//Take inverse cosine (arc-cosine)  of cos (theta) to get Theta
	double Theta = FMath::Acos(CosTheta);
	//convert from radians to degrees
	Theta = FMath::RadiansToDegrees(Theta);

	const FVector CrossProduct = FVector::CrossProduct(Forward, HitDist);
	if (CrossProduct.Z < 0)
	{
		Theta *= -1.f;
	}

	FName Section("HitB");

	if (Theta >= -45.f && Theta < 45.f)
	{
		Section = FName("HitF");
	}
	else if (Theta >= -135.f && Theta < -45.f)
	{
		Section = FName("HitL");
	}
	else if (Theta >= 45.f && Theta < 135.f)
	{
		Section = FName("HitR");
	}

	PlayHitReactMontage(Section);
}

void ARNPC::HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bHitReacting = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed = bHitReacting ? 0.f : BaseWalkSpeed;
	//AIC->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), bHitReacting);
}

AActor* ARNPC::GetCombatTarget_Implementation() const
{
	return CombatTarget;
}

FVector ARNPC::GetCombatSocketLocation_Implementation()
{
	return Weapon->GetSocketLocation(DamageSocket);
}

void ARNPC::SetCombatTarget_Implementation(AActor* InCombatTarget)
{
	CombatTarget = InCombatTarget;
}

void ARNPC::GetHit_Implementation(const FVector& ImpactPoint)
{
	DrawDebugSphere(GetWorld(), ImpactPoint, 10.f, 12, FColor::Cyan, false, 5.f);

	DirectionalHitReact(ImpactPoint);

	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, ImpactPoint);
	}
	if (HitParticles)
	{
		UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitParticles, ImpactPoint);
	}
}

int32 ARNPC::GetCharacterLevel_Implementation()
{
	return Level;
}





void ARNPC::PlayHitReactMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReact)
	{
		AnimInstance->Montage_Play(HitReact);
		AnimInstance->Montage_JumpToSection(SectionName, HitReact);
	}
}