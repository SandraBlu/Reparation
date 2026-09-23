// Fill out your copyright notice in the Description page of Project Settings.
#define LOCTEXT_NAMESPACE "EquipItem"

#include "Items/REffectItem.h"

UREffectItem::UREffectItem()
{
	bStackable = true;
	Weight = .25f;
	UseActionText = LOCTEXT("ItemUseActionText", "Ingest");
}

#undef LOCTEXT_NAMESPACE
