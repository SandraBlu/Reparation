// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RNPC.h"
#include "Components/WidgetComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ARNPC::ARNPC()
{
	GetCharacterMovement()->bUseControllerDesiredRotation = true;

	//Health = CreateDefaultSubobject<UWidgetComponent>("Health");
	//Health->SetupAttachment(GetRootComponent());

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

void ARNPC::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
}

void ARNPC::PossessedBy(AController* NewController)
{

}
