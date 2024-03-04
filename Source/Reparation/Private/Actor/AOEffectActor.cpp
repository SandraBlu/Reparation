// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/AOEffectActor.h"
#include "Components/SphereComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Attributes/AOAttributeSet.h"

// Sets default values
AAOEffectActor::AAOEffectActor()
{
	PrimaryActorTick.bCanEverTick = false;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	SetRootComponent(Mesh);
	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	Sphere->SetupAttachment(GetRootComponent());
}

void AAOEffectActor::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(OtherActor))
	{
		const UAOAttributeSet* AOAttributeSet = Cast<UAOAttributeSet>(ASCInterface->GetAbilitySystemComponent()->GetAttributeSet(UAOAttributeSet::StaticClass()));
		UAOAttributeSet* MutableAttrSet = const_cast<UAOAttributeSet*>(AOAttributeSet);
		MutableAttrSet->SetStamina(AOAttributeSet->GetStamina() - 25.f);
		Destroy();
	}
}

void AAOEffectActor::EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}

void AAOEffectActor::BeginPlay()
{
	Super::BeginPlay();
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AAOEffectActor::OnOverlap);
	Sphere->OnComponentEndOverlap.AddDynamic(this, &AAOEffectActor::EndOverlap);
}
