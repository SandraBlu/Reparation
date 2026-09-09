// Fill out your copyright notice in the Description page of Project Settings.


#include "NavigatorEditorBFL.h"

#include "Factories/Texture2dFactoryNew.h"

UTexture2DFactoryNew* UNavigatorEditorBFL::GetTexture2DFactory()
{
	return NewObject<UTexture2DFactoryNew>();
}
