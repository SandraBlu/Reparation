// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/RMelee.h"

#include "Components/BoxComponent.h"
#include "Interfaces/RCombatInterface.h"
#include "Kismet/KismetSystemLibrary.h"

ARMelee::ARMelee()
{
	WeaponBox = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponBox"));
	WeaponBox->SetupAttachment(GetRootComponent());

	TraceStart = CreateDefaultSubobject<USceneComponent>(TEXT("TraceStart"));
	TraceStart->SetupAttachment(GetRootComponent());

	TraceEnd = CreateDefaultSubobject<USceneComponent>(TEXT("TraceEnd"));
	TraceEnd->SetupAttachment(GetRootComponent());
}

void ARMelee::OnBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                           int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//GetWorld Location Of Start and End Scene Components
	const FVector Start = TraceStart->GetComponentLocation();
	const FVector End = TraceEnd->GetComponentLocation();
	//Add actors to Ignore Array: this = weapon
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	for (AActor* Actor : IgnoreActors)
	{
		ActorsToIgnore.AddUnique(Actor);
	}

	FHitResult BoxHit;

	UKismetSystemLibrary::BoxTraceSingle(this, Start, End, FVector(5.f, 5.f, 5.f), TraceStart->GetComponentRotation(), ETraceTypeQuery::TraceTypeQuery1, false,
		ActorsToIgnore, EDrawDebugTrace::ForDuration, BoxHit, true);

	if (BoxHit.GetActor())
	{
		if (IRCombatInterface* iHit = Cast<IRCombatInterface>(BoxHit.GetActor()))
		{
			iHit->Execute_GetHit(BoxHit.GetActor(), BoxHit.ImpactPoint);
			OnHit(BoxHit);
		}
		IgnoreActors.AddUnique(BoxHit.GetActor());
		
		CreateForceFields(BoxHit.ImpactPoint);
	}
}

void ARMelee::BeginPlay()
{
	Super::BeginPlay();
	WeaponBox->OnComponentBeginOverlap.AddDynamic(this, &ARMelee::OnBoxOverlap);
}
