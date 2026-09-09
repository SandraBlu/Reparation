// Fill out your copyright notice in the Description page of Project Settings.


#include "Navigation/POI.h"

#include "Components/SphereComponent.h"
#include "Navigation/NavigationComponent.h"
#include "Navigation/POINavMarker.h"

#define LOCTEXT_NAMESPACE "PointOfInterest"

// Sets default values
APOI::APOI()
{
	POISphere = CreateDefaultSubobject<USphereComponent>("POISphere");
	POISphere->SetSphereRadius(500.f);
	POISphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	POISphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SetRootComponent(POISphere);

	POIDisplayName = LOCTEXT("POIDisplayName", "Point of Interest");

	NavigationMarker = CreateDefaultSubobject<UPOINavMarker>("NavigationMarker");
	NavigationMarker->DefaultMarkerSettings.LocationDisplayName = POIDisplayName;

}
#if WITH_EDITOR
void APOI::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(APOI, POIDisplayName))
	{
		if (NavigationMarker)
		{
			NavigationMarker->DefaultMarkerSettings.LocationDisplayName = POIDisplayName;
		}
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif 

void APOI::BeginPlay()
{
	Super::BeginPlay();

	//On beginplay we won't be discovered, when player loads they will iterate us and set discovery 
	SetDiscovered(false);
}

void APOI::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (HasAuthority())
	{
		if (APawn* Pawn = Cast<APawn>(OtherActor))
		{
			if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
			{
				if (UNavigationComponent* NavComp = Cast<UNavigationComponent>(PC->GetComponentByClass(UNavigationComponent::StaticClass())))
				{
					NavComp->HandleEnterPOI(this);
				}
			}
		}
	}
}

void APOI::SetDiscovered(const bool bDiscovered)
{
	if (NavigationMarker)
	{
		NavigationMarker->SetDiscovered(bDiscovered);
	}
}

#undef LOCTEXT_NAMESPACE 

