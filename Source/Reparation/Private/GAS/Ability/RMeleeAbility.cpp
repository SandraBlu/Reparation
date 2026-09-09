// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/RMeleeAbility.h"

#include "AI/REnemy.h"

class UPawnCombatComponent* URMeleeAbility::GetCombatComponentFromActorInfo()
{
	return GetEnemyCharacterFromActorInfo()->GetPawnCombatComponent();
}
