// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/Data/CharacterClassInfo.h"

FCharacterDefaultInfo UCharacterClassInfo::GetClassDefaultInfo(ECharacterClass CharacterClass)
{
	return CharClassInfo.FindChecked(CharacterClass);
}
