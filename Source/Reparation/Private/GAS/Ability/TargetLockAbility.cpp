// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/TargetLockAbility.h"

#include "EnhancedInputSubsystems.h"
#include "RGameplayTags.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Characters/RPlayer.h"
#include "Components/SizeBox.h"
#include "Framework/RPlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/CombatWidget.h"

void UTargetLockAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	TryLockOnTarget();
	InitTargetLockMovement();
	InitTargetLockMappingContext();
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UTargetLockAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	ResetTargetLockMovement();
	ResetTargetLockMappingContext();
	CleanUp();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UTargetLockAbility::OnTargetLockTick(float DeltaTime)
{
	const FRGameplayTags GameplayTags = FRGameplayTags::Get();
	if (!CurrentLockedActor || URAbilitySystemLibrary::NativeDoesActorHaveTag(CurrentLockedActor,GameplayTags.Event_Death) ||
		URAbilitySystemLibrary::NativeDoesActorHaveTag(GetPlayerFromActorInfo(), GameplayTags.Event_Death))
	{
		CancelTargetLockAbility();
	}
	SetTargetLockWidgetPosition();

	const bool bShouldOverrideRotation = !URAbilitySystemLibrary::NativeDoesActorHaveTag(GetPlayerFromActorInfo(),GameplayTags.Event_Evading)
	&& !URAbilitySystemLibrary::NativeDoesActorHaveTag(GetPlayerFromActorInfo(),GameplayTags.Event_Blocking);

	if (bShouldOverrideRotation)
	{
		const FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(GetPlayerFromActorInfo()->GetActorLocation(),CurrentLockedActor->GetActorLocation());
		const FRotator CurrentControlRot = GetPlayerControllerFromActorInfo()->GetControlRotation();
		const FRotator TargetRot = FMath::RInterpTo(CurrentControlRot,LookAtRot,DeltaTime,TargetLockRotationInterpSpeed);

		GetPlayerControllerFromActorInfo()->SetControlRotation(FRotator(TargetRot.Pitch,TargetRot.Yaw,0.f));
		GetPlayerFromActorInfo()->SetActorRotation(FRotator(0.f,TargetRot.Yaw,0.f));
	}
}

void UTargetLockAbility::SwitchTarget(const FGameplayTag& InSwitchDirectionTag)
{
	const FRGameplayTags GameplayTags = FRGameplayTags::Get();
	GetAvailableActorsToLockOnto();
	TArray<AActor*> ActorsOnLeft;
	TArray<AActor*> ActorsOnRight;
	AActor* NewTargetToLock = nullptr;
	GetAvailableActorsAroundTarget(ActorsOnLeft,ActorsOnRight);

	if (InSwitchDirectionTag == GameplayTags.Event_SwitchTarget_Left)
	{
		NewTargetToLock = GetNearestTargetFromAvailableActors(ActorsOnLeft);
	}
	else
	{
		NewTargetToLock = GetNearestTargetFromAvailableActors(ActorsOnRight);
	}

	if (NewTargetToLock)
	{
		CurrentLockedActor = NewTargetToLock;
	}
	
}

void UTargetLockAbility::TryLockOnTarget()
{
	GetAvailableActorsToLockOnto();
	if (AvailableTargetLockActors.IsEmpty())
	{
		CancelTargetLockAbility();
		return;
	}
	CurrentLockedActor = GetNearestTargetFromAvailableActors(AvailableTargetLockActors);

	if (CurrentLockedActor)
	{
		DrawTargetLockWidget();
		SetTargetLockWidgetPosition();
	}
	else
	{
		CancelTargetLockAbility();
	}
}

void UTargetLockAbility::GetAvailableActorsToLockOnto()
{
	AvailableTargetLockActors.Empty();
	TArray<FHitResult> BoxTraceHits;

	UKismetSystemLibrary::BoxTraceMultiForObjects(GetPlayerFromActorInfo(),GetPlayerFromActorInfo()->GetActorLocation(),
		GetPlayerFromActorInfo()->GetActorLocation() + GetPlayerFromActorInfo()->GetActorForwardVector() * BoxTraceDistance, TraceBoxSize / 2.f,
		GetPlayerFromActorInfo()->GetActorForwardVector().ToOrientationRotator(),BoxTraceChannel,false,TArray<AActor*>(),
		bShowPersistentDebugShape? EDrawDebugTrace::Persistent : EDrawDebugTrace::None,BoxTraceHits,true);

	for (const FHitResult& TraceHit : BoxTraceHits)
	{
		if (AActor* HitActor = TraceHit.GetActor())
		{
			if (HitActor != GetPlayerFromActorInfo())
			{
				AvailableTargetLockActors.AddUnique(HitActor);
				//Debug::Print(HitActor->GetActorNameOrLabel());
			}
		}
	}
}

AActor* UTargetLockAbility::GetNearestTargetFromAvailableActors(const TArray<AActor*>& InAvailableActors)
{
	float ClosestDistance = 0.f;
	return UGameplayStatics::FindNearestActor(GetPlayerFromActorInfo()->GetActorLocation(),InAvailableActors,ClosestDistance);
}

void UTargetLockAbility::GetAvailableActorsAroundTarget(TArray<AActor*>& OutActorsOnLeft,
	TArray<AActor*>& OutActorsOnRight)
{
	if (!CurrentLockedActor || AvailableTargetLockActors.IsEmpty())
	{
		CancelTargetLockAbility();
		return;
	}

	const FVector PlayerLocation = GetPlayerFromActorInfo()->GetActorLocation();
	const FVector PlayerToCurrentNormalized = (CurrentLockedActor->GetActorLocation() - PlayerLocation).GetSafeNormal();

	for (AActor* AvailableActor : AvailableTargetLockActors)
	{
		if(!AvailableActor || AvailableActor == CurrentLockedActor) continue;

		const FVector PlayerToAvailableNormalized = (AvailableActor->GetActorLocation() - PlayerLocation).GetSafeNormal();

		const FVector CrossResult = FVector::CrossProduct(PlayerToCurrentNormalized,PlayerToAvailableNormalized);

		if (CrossResult.Z>0.f)
		{
			OutActorsOnRight.AddUnique(AvailableActor);
		}
		else
		{
			OutActorsOnLeft.AddUnique(AvailableActor);
		}
	}
}

void UTargetLockAbility::CancelTargetLockAbility()
{
	CancelAbility(GetCurrentAbilitySpecHandle(),GetCurrentActorInfo(),GetCurrentActivationInfo(),true);
}

void UTargetLockAbility::CleanUp()
{
	AvailableTargetLockActors.Empty();

	CurrentLockedActor = nullptr;
	if (DrawnTargetLockWidget)
	{
		DrawnTargetLockWidget->RemoveFromParent();
	}
	DrawnTargetLockWidget = nullptr;
	TargetLockWidgetSize = FVector2D::ZeroVector;
	DefaultMaxWalkSpeed = 0.f;
}

void UTargetLockAbility::InitTargetLockMovement()
{
	DefaultMaxWalkSpeed = GetPlayerFromActorInfo()->GetCharacterMovement()->MaxWalkSpeed;
	GetPlayerFromActorInfo()->GetCharacterMovement()->MaxWalkSpeed = TargetLockWalkSpeed;
}

void UTargetLockAbility::InitTargetLockMappingContext()
{
	const ULocalPlayer* LocalPlayer = GetPlayerControllerFromActorInfo()->GetLocalPlayer();
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	check(Subsystem)
	Subsystem->AddMappingContext(TargetLockMappingContext, 3);
}

void UTargetLockAbility::ResetTargetLockMovement()
{
	if (DefaultMaxWalkSpeed > 0.f)
	{
		GetPlayerFromActorInfo()->GetCharacterMovement()->MaxWalkSpeed = DefaultMaxWalkSpeed;
	}
}

void UTargetLockAbility::ResetTargetLockMappingContext()
{
	if (!GetPlayerControllerFromActorInfo())
	{
		return; //PlayerController is not valid, so we can't reset the mapping context.
	}
	const ULocalPlayer* LocalPlayer = GetPlayerControllerFromActorInfo()->GetLocalPlayer();
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	check(Subsystem)
	Subsystem->RemoveMappingContext(TargetLockMappingContext);
}

void UTargetLockAbility::DrawTargetLockWidget()
{
	if (!DrawnTargetLockWidget)
	{
		checkf(TargetLockWidgetClass, TEXT("Forgot to assign a valid widget class in Blueprint"));
		DrawnTargetLockWidget = CreateWidget<UCombatWidget>(GetWorld(), TargetLockWidgetClass);
		check(DrawnTargetLockWidget);
		DrawnTargetLockWidget->AddToViewport();
	}
}

void UTargetLockAbility::SetTargetLockWidgetPosition()
{
	if (!DrawnTargetLockWidget  || !CurrentLockedActor)
	{
		CancelTargetLockAbility();
		return;
	}

	FVector2D ScreenPosition;
	UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(GetPlayerControllerFromActorInfo(),CurrentLockedActor->GetActorLocation(),ScreenPosition,true);

	if (TargetLockWidgetSize == FVector2D::ZeroVector)
	{
		DrawnTargetLockWidget->WidgetTree->ForEachWidget([this](UWidget* FoundWidget)
			{
				if (USizeBox* FoundSizeBox = Cast<USizeBox>(FoundWidget))
				{
					TargetLockWidgetSize.X = FoundSizeBox->GetWidthOverride();
					TargetLockWidgetSize.Y = FoundSizeBox->GetHeightOverride();
				}
			}
		);
	}

	ScreenPosition -= (TargetLockWidgetSize / 2.f);

	DrawnTargetLockWidget->SetPositionInViewport(ScreenPosition,false);
}


