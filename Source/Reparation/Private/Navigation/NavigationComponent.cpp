// Fill out your copyright notice in the Description page of Project Settings.


#include "Navigation/NavigationComponent.h"

#include "Navigation/CustomWaypoint.h"
#include "Navigation/MapTileBounds.h"
#include "Navigation/MapTileComponent.h"
#include "Navigation/POI.h"
#include "Net/UnrealNetwork.h"
#include "EngineUtils.h"


UNavigationComponent::UNavigationComponent()
{
	SetIsReplicatedByDefault(true);
	MapWidth = 5000.f;
	MapOrigin = FVector2D::ZeroVector;
	MaxCustomWaypoints = 5;
}

void UNavigationComponent::BeginPlay()
{
	Super::BeginPlay();

	//Try pull our navdata from the maptilebounds should one exist 
	for (TActorIterator<AMapTileBounds> It(GetWorld()); It; ++It)
	{
		MapTileBounds = *It;

		if (MapTileBounds)
		{
			MapWidth = MapTileBounds->MapWidth;
			MapOrigin = FVector2D(MapTileBounds->GetActorLocation());

			break;
		}
	}
	
}

void UNavigationComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UNavigationComponent, DiscoveredPOIs, COND_OwnerOnly);
}

bool UNavigationComponent::AddMarker(class UNavMarkerComponent* NavMarker)
{
	if (NavMarker)
	{
		Markers.Add(NavMarker);
		OnMarkerAdded.Broadcast(NavMarker);
		return true;
	}

	return false;
}

bool UNavigationComponent::RemoveMarker(class UNavMarkerComponent* NavMarker)
{
	if (NavMarker)
	{
		Markers.Remove(NavMarker);
		OnMarkerRemoved.Broadcast(NavMarker);
		return true;
	}
	return false;
}

void UNavigationComponent::ServerSelectMarker_Implementation(class UNavMarkerComponent* Marker)
{
	SelectMarker(Marker);
}

void UNavigationComponent::SelectMarker(class UNavMarkerComponent* Marker)
{
	if (APlayerController* OwnerPC = Cast<APlayerController>(GetOwner()))
	{
		if (Marker && Marker->CanInteract(this, OwnerPC))
		{
			if (GetOwnerRole() < ROLE_Authority)
			{
				ServerSelectMarker(Marker);
			}

			Marker->OnSelect(this, OwnerPC);
			Marker->OnSelected.Broadcast(this, OwnerPC);
		}
	}
}

bool UNavigationComponent::SetMapLayer(FGameplayTag NewLayer, FGameplayTagContainer Domains)
{
	for (auto& MapTile : MapTiles)
	{
		if (MapTile)
		{
			const bool bSucceeded = MapTile->SetMapLayer(NewLayer, Domains);

			if (!bSucceeded)
			{
				return false;
			}
		}
	}

	return true;
}

bool UNavigationComponent::HasDiscoveredPOI(const FGameplayTagContainer& POITag) const
{
	return DiscoveredPOIs.HasAll(POITag);
}

bool UNavigationComponent::HasDiscoveredPOI(class APOI* POI) const
{
	if (POI)
	{
		return HasDiscoveredPOI(POI->POITags);
	}

	return false;
}

void UNavigationComponent::HandleEnterPOI(class APOI* POI)
{
	if (GetOwnerRole() >= ROLE_Authority && POI)
	{	
		if (!HasDiscoveredPOI(POI))
		{
			DiscoveredPOIs.AppendTags(POI->POITags);
			SetPOIDiscovered(POI);
		}
	}
}

void UNavigationComponent::ClientSetPOIDiscovered_Implementation(class APOI* POI)
{
	if (POI)
	{
		POI->SetDiscovered(true);
		OnPOIDiscovered.Broadcast(POI);
	}
}


void UNavigationComponent::SetPOIDiscovered(class APOI* POI)
{
	if (GetOwnerRole() >= ROLE_Authority && POI)
	{
		POI->SetDiscovered(true);
		OnPOIDiscovered.Broadcast(POI);

		if (GetNetMode() != NM_Standalone)
		{
			ClientSetPOIDiscovered(POI);
		}
	}
}

ACustomWaypoint* UNavigationComponent::PlaceCustomWaypoint(const FTransform& Transform)
{
	if (CustomWaypoints.Num() + 1 <= MaxCustomWaypoints)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.bNoFail = true;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.Owner = GetOwner();

		if (ACustomWaypoint* Waypoint = GetWorld()->SpawnActor<ACustomWaypoint>(ACustomWaypoint::StaticClass(), Transform, SpawnParams))
		{
			CustomWaypoints.AddUnique(Waypoint);

			return Waypoint;
		}
	}

	return nullptr;
}

void UNavigationComponent::RemoveCustomWaypoint(class ACustomWaypoint* Waypoint)
{
	if (Waypoint)
	{
		CustomWaypoints.Remove(Waypoint);
		Waypoint->Destroy();
	}
}

//bool UNavigationComponent::Save(const FString& SaveName /*= "NarrativeNavigatorSaveData"*/, const int32 Slot /*= 0*/)
/*{
	if (UMySaveGame* NavigatorSave = Cast<UMySaveGame>(UGameplayStatics::CreateSaveGameObject(UMySaveGame::StaticClass())))
	{
		for (auto& Waypoint : CustomWaypoints)
		{
			if (Waypoint)
			{
				NavigatorSave->CustomMarkerTransforms.Add(Waypoint->GetActorTransform());
			}
		}

		NavigatorSave->DiscoveredPOIs = DiscoveredPOIs;

		if (UGameplayStatics::SaveGameToSlot(NavigatorSave, SaveName, Slot))
		{
			return true;
		}
	}
	return true;
}

/*bool UNavigationComponent::Load(const FString& SaveName /*= "NarrativeNavigatorSaveData"*///, const int32 Slot /*= 0*/)
/*{
	if (!UGameplayStatics::DoesSaveGameExist(SaveName, 0))
	{
		return false;
	}

	//Destroy our existings 
	for (auto& Waypoint : CustomWaypoints)
	{
		if (Waypoint)
		{
			Waypoint->Destroy();
		}
	}
	CustomWaypoints.Empty();

	if (URSaveGame* NavigatorSave = Cast<UMySaveGame>(UGameplayStatics::LoadGameFromSlot(SaveName, Slot)))
	{
		for (auto& MarkerTransform : NavigatorSave->CustomMarkerTransforms)
		{
			PlaceCustomWaypoint(MarkerTransform);
		}

		DiscoveredPOIs = NavigatorSave->DiscoveredPOIs;

		for (TActorIterator<APOI> It(GetWorld()); It; ++It)
		{
			if (It)
			{
				if (APOI* POI = *It)
				{
					POI->SetDiscovered(HasDiscoveredPOI(POI));
				}
			}
		}
		return true;
	}

	return false;
}*/

