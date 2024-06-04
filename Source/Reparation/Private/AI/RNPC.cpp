// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RNPC.h"

ARNPC::ARNPC()
{
	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	Weapon->SetupAttachment(GetMesh(), FName("weapon"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
}
