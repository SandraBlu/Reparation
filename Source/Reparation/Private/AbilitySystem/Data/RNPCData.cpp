// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Data/RNPCData.h"

FNPCClassInfo URNPCData::GetClassInfo(ENPCClass NPCClass)
{
	return NPCClassInfo.FindChecked(NPCClass);
}
