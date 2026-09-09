// Fill out your copyright notice in the Description page of Project Settings.


#include "Navigation/NavigatorBFL.h"

#include "Navigation/NavMarkerComponent.h"

UNavMarkerComponent* UNavigatorBFL::AddNavigationMarkerToActor(class AActor* ActorToMark, const FNavigationMarkerSettings& MarkerSettings, const FGameplayTagContainer& InMarkerDomain)
{
	if (ActorToMark)
	{
		UNavMarkerComponent* Marker = Cast<UNavMarkerComponent>(ActorToMark->AddComponentByClass(UNavMarkerComponent::StaticClass(), false, FTransform(), false));
		if (Marker)
		{
			Marker->DefaultMarkerSettings = MarkerSettings;
			Marker->SetDomain(InMarkerDomain);

			return Marker;
		}
	}

	return nullptr;
}