// Fill out your copyright notice in the Description page of Project Settings.


#include "Navigation/CustomWaypointMarker.h"

#include "RGameplayTags.h"
#include "Navigation/CustomWaypoint.h"
#include "Navigation/NavigationComponent.h"

UCustomWaypointMarker::UCustomWaypointMarker()
{
	DefaultMarkerSettings.LocationDisplayName = NSLOCTEXT("CustomWaypointMarker", "MarkerDisplayName", "Custom Waypoint");
	DefaultMarkerSettings.IconTint = FLinearColor(0.178507f, 0.538802f, 0.859375f,1.000000f);

	bPinToMapEdge = true;

	MarkerDomain.AddTag(FRGameplayTags::Get().NavigatorTypes_Screenspace);
}

FText UCustomWaypointMarker::GetMarkerActionText_Implementation(class UNavigationComponent* Selector, class APlayerController* SelectorOwner) const
{
	return NSLOCTEXT("CustomWaypointMarker", "MarkerActionText", "Remove");
}

void UCustomWaypointMarker::OnSelect_Implementation(class UNavigationComponent* Selector, class APlayerController* SelectorOwner)
{
	Super::OnSelect_Implementation(Selector, SelectorOwner);
	if (Selector)
	{
		if (ACustomWaypoint* Waypoint = Cast<ACustomWaypoint>(GetOwner()))
		{
			Selector->RemoveCustomWaypoint(Waypoint);
		}
	}
}
