// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTT_RangedAttack.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTT_RangedAttack::UBTT_RangedAttack()
{
	MaxProjectileSpread = 2.f;
}

EBTNodeResult::Type UBTT_RangedAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (IsValid(AIController))
	{
		ACharacter* AIPawn = Cast<ACharacter>(AIController->GetPawn());
		if (AIPawn == nullptr)
		{
			return EBTNodeResult::Failed;
		}
		FVector WeaponLocation = AIPawn->GetMesh()->GetSocketLocation("weapon_01");

		AActor* TargetActor = Cast<AActor>(BBComp->GetValueAsObject("TargetActor"));
		if (TargetActor == nullptr)
		{
			return EBTNodeResult::Failed;
		}
		/*	if (!UAOAttributeSet::(TargetActor))
			{
				return EBTNodeResult::Failed;
			}*/

		FVector Direction = TargetActor->GetActorLocation() - WeaponLocation;
		FRotator WeaponRotation = Direction.Rotation();

		WeaponRotation.Pitch += FMath::RandRange(0.f, MaxProjectileSpread);
		WeaponRotation.Yaw += FMath::RandRange(-MaxProjectileSpread, MaxProjectileSpread);

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Params.Instigator = AIPawn;

		AActor* NewProj = GetWorld()->SpawnActor<AActor>(ProjectileClass, WeaponLocation, WeaponRotation, Params);

		return NewProj ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
	}
	return EBTNodeResult::Failed;;
}
