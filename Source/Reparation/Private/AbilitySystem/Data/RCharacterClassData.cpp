// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Data/RCharacterClassData.h"

FCharClassInfo URCharacterClassData::GetClassInfo(ECharacterClass CharacterClass)
{
	return CharClassInfo.FindChecked(CharacterClass);
}
