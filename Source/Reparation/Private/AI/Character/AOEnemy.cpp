// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Character/AOEnemy.h"

AAOEnemy::AAOEnemy()
{

}

void AAOEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (ASC)
		ASC->InitAbilityActorInfo(this, this);
}
