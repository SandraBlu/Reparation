// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Data/RCharacterClassInfo.h"

FCharClassInfo URCharacterClassInfo::GetClassInfo(ECharacterClass CharacterClass)
{
	return CharacterClassInfo.FindChecked(CharacterClass);
}
