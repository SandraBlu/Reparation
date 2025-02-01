// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/RMeleeAbility.h"

#include "AI/REnemy.h"

class AREnemy* URMeleeAbility::GetEnemyCharacterFromActorInfo()
{
	if (!CachedWarriorEnemyCharacter.IsValid())
	{
		CachedWarriorEnemyCharacter = Cast<AREnemy>(CurrentActorInfo->AvatarActor);
	}
	return CachedWarriorEnemyCharacter.IsValid()? CachedWarriorEnemyCharacter.Get() : nullptr;
}

class UREnemyCombatComponent* URMeleeAbility::GetEnemyCombatComponentFromActorInfo()
{
	return GetEnemyCharacterFromActorInfo()->GetEnemyCombatComponent();
}