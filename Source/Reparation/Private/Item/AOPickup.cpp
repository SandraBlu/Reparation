// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/AOPickup.h"
#include "Components/AOInteractComponent.h"
#include "NiagaraComponent.h"
#include "Item/AOItem.h"
#include "Components/AOInventoryComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Character/AOPlayerBase.h"

// Sets default values
AAOPickup::AAOPickup()
{
	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>("PickupMesh");
	PickupMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	SetRootComponent(PickupMesh);

	PickupVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PickupVFX"));
	PickupVFX->SetupAttachment(GetRootComponent());

	InteractComp = CreateDefaultSubobject<UAOInteractComponent>("PickupInteractComp");
	InteractComp->InteractTime = 0.f;
	InteractComp->InteractDistance = 750.f;
	InteractComp->InteractItemName = FText::FromString("Pickup");
	InteractComp->InteractActionText = FText::FromString("Take");
	InteractComp->OnInteract.AddDynamic(this, &AAOPickup::OnTakePickup);
	InteractComp->SetupAttachment(PickupMesh);

}

void AAOPickup::InitializePickup(const TSubclassOf<class UAOItem> ItemClass, const int32 Quantity)
{
	if (ItemClass && Quantity > 0)
	{
		Item = NewObject<UAOItem>(this, ItemClass);
		Item->SetQuantity(Quantity);

		UpdateItemSpecs();
	}
}

void AAOPickup::BeginPlay()
{
	Super::BeginPlay();

	if (ItemTemplate && bNetStartup)
	{
		InitializePickup(ItemTemplate->GetClass(), ItemTemplate->GetQuantity());
	}
	//if pickup is dropped during game on 20 degree slope, it aligns with ground on respawn on 20 degree slope
	if (!bNetStartup)
	{
		AlignWithGround();
	}
}

void AAOPickup::UpdateItemSpecs()
{
	if (Item)
	{
		PickupMesh->SetStaticMesh(Item->PickupMesh);
		InteractComp->InteractItemName = Item->ItemDisplayName;

		//Bind to delegate
		Item->OnItemModified.AddDynamic(this, &AAOPickup::OnItemModified);
	}
	InteractComp->RefreshWidget();
}

void AAOPickup::OnItemModified()
{

}

#if WITH_EDITOR
void AAOPickup::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	//if new pickup is selected in property editor, change mesh to reflect the new item being selected
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AAOPickup, ItemTemplate))
	{
		if (ItemTemplate)
		{
			PickupMesh->SetStaticMesh(ItemTemplate->PickupMesh);
		}
	}
}
#endif

void AAOPickup::OnTakePickup(class AAOPlayerBase* Taker)
{
	if (!Taker)
	{
		UE_LOG(LogTemp, Warning, TEXT("Pickup was taken but player was not valid."));
		return;
	}

	if (!IsPendingKillPending() && Item)
	{
		if (UAOInventoryComponent* PlayerInventory = Taker->PlayerInventory)
		{
			const FItemAddResult AddResult = PlayerInventory->TryAddItem(Item);
			//Don't destroy pickup if we have only taken part of the item on the ground; say 2 out of 6
			if (AddResult.ActualAmountGiven < Item->GetQuantity())
			{
				Item->SetQuantity(Item->GetQuantity() - AddResult.ActualAmountGiven);
			}
			//otherwise take all and destroy item
			else if (AddResult.ActualAmountGiven >= Item->GetQuantity())
			{
				if (Item->PickupSFX)
				{
					UGameplayStatics::PlaySoundAtLocation(this, Item->PickupSFX, GetActorLocation());
				}
				Destroy();
			}
		}
	}
}