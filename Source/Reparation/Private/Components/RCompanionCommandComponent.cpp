#include "Components/RCompanionCommandComponent.h"
#include "Components/RCompanionPartyComponent.h"
#include "Characters/RPlayer.h"
#include "AI/RCompanion.h"
#include "AI/AIC_Companion.h"
#include "UI/RCompanionCommandWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Locomotion/RLocomotionComponent.h"
#include "NavigationSystem.h"
#include "Components/TextRenderComponent.h"

URCompanionCommandComponent::URCompanionCommandComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	WidgetClass = URCompanionCommandWidget::StaticClass();
}

void URCompanionCommandComponent::SetupInput(UEnhancedInputComponent* Input)
{
	ARPlayer* Player = Cast<ARPlayer>(GetOwner());
	APlayerController* PC = Player ? Cast<APlayerController>(Player->GetController()) : nullptr;
	if (!Input || !PC || !PC->IsLocalController() || !PC->GetLocalPlayer()) return;
	Cancel();
	if (BoundInput.IsValid()) for (uint32 Handle : BindingHandles) BoundInput->RemoveBindingByHandle(Handle);
	BindingHandles.Reset();
	if (InputSubsystem.IsValid()) InputSubsystem->RemoveMappingContext(OpenContext);
	BoundInput = Input;
	InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	if (!InputSubsystem.IsValid()) return;
	if (!OpenContext)
	{
		OpenContext = NewObject<UInputMappingContext>(this, TEXT("IMC_CompanionCommands"));
		ActiveContext = NewObject<UInputMappingContext>(this, TEXT("IMC_CompanionCommandsActive"));
		OpenAction = NewObject<UInputAction>(this);
		CancelAction = NewObject<UInputAction>(this);
		ConfirmAction = NewObject<UInputAction>(this);
		NextAction = NewObject<UInputAction>(this);
		PreviousAction = NewObject<UInputAction>(this);
		ActiveContext->MapKey(CancelAction, EKeys::Escape);
		ActiveContext->MapKey(ConfirmAction, EKeys::LeftMouseButton);
		ActiveContext->MapKey(NextAction, EKeys::MouseScrollDown);
		ActiveContext->MapKey(PreviousAction, EKeys::MouseScrollUp);
	}
	OpenContext->UnmapAll();
	OpenContext->MapKey(OpenAction, MenuKey);
	BindingHandles.Add(Input->BindAction(OpenAction, ETriggerEvent::Started, this, &ThisClass::OpenMenu).GetHandle());
	BindingHandles.Add(Input->BindAction(OpenAction, ETriggerEvent::Completed, this, &ThisClass::ReleaseMenu).GetHandle());
	BindingHandles.Add(Input->BindAction(OpenAction, ETriggerEvent::Canceled, this, &ThisClass::Cancel).GetHandle());
	BindingHandles.Add(Input->BindAction(CancelAction, ETriggerEvent::Started, this, &ThisClass::Cancel).GetHandle());
	BindingHandles.Add(Input->BindAction(ConfirmAction, ETriggerEvent::Started, this, &ThisClass::ConfirmTarget).GetHandle());
	BindingHandles.Add(Input->BindAction(NextAction, ETriggerEvent::Started, this, &ThisClass::NextCompanion).GetHandle());
	BindingHandles.Add(Input->BindAction(PreviousAction, ETriggerEvent::Started, this, &ThisClass::PreviousCompanion).GetHandle());
	InputSubsystem->AddMappingContext(OpenContext, 50);
}

void URCompanionCommandComponent::OpenMenu()
{
	ARPlayer* Player = Cast<ARPlayer>(GetOwner());
	APlayerController* PC = Player ? Cast<APlayerController>(Player->GetController()) : nullptr;
	if (!PC || !Player->CompanionParty || !InputSubsystem.IsValid() || !WidgetClass) return;
	if (IsCommandInputActive()) Cancel();
	if (!IsValid(Player->CompanionParty->SelectedCompanion)) CycleCompanion(1);
	if (!IsValid(Player->CompanionParty->SelectedCompanion)) return;
	Widget = CreateWidget<URCompanionCommandWidget>(PC, WidgetClass);
	if (!Widget) return;
	MenuController = PC;
	bSavedCursor = PC->bShowMouseCursor;
	bMenuOpen = true;
	PC->SetIgnoreMoveInput(true);
	PC->SetIgnoreLookInput(true);
	bMovementBlocked = bLookBlocked = true;
	if (Player->Locomotion) Player->Locomotion->SetMovementInput(FVector2D::ZeroVector);
	PC->bShowMouseCursor = true;
	int32 Width, Height;
	PC->GetViewportSize(Width, Height);
	PC->SetMouseLocation(Width / 2, Height / 2);
	Widget->SetVisibility(ESlateVisibility::HitTestInvisible);
	Widget->AddToPlayerScreen(100);
	Widget->Feedback = NSLOCTEXT("CompanionUI", "Help", "Point to a command; release to confirm. Scroll: companion");
	UpdateLabel();
	FModifyContextOptions Options;
	Options.bIgnoreAllPressedKeysUntilRelease = false;
	InputSubsystem->AddMappingContext(ActiveContext, 100, Options);
	SetComponentTickEnabled(true);
}

void URCompanionCommandComponent::ReleaseMenu()
{
	if (!bMenuOpen || !Widget) return;
	ARPlayer* Player = Cast<ARPlayer>(GetOwner());
	if (!Player || !Player->CompanionParty) { Cancel(); return; }
	const ERCompanionMenuChoice Choice = Widget->Choice;
	if (Choice == ERCompanionMenuChoice::GoToLocation || Choice == ERCompanionMenuChoice::Attack)
	{
		if (Choice == ERCompanionMenuChoice::Attack && (!IsValid(Player->CompanionParty->SelectedCompanion) || !Player->CompanionParty->SelectedCompanion->IsCombatReady()))
		{
			Widget->Feedback = IsValid(Player->CompanionParty->SelectedCompanion) ?
				Player->CompanionParty->SelectedCompanion->GetCombatReadinessIssue() : NSLOCTEXT("CompanionUI", "NoCompanion", "No companion selected.");
			return;
		}
		TargetRecipient = Player->CompanionParty->SelectedCompanion;
		bMenuOpen = false;
		bTargeting = true;
		bAttackTargeting = Choice == ERCompanionMenuChoice::Attack;
		Widget->bTargeting = true;
		Widget->bAttackTargeting = bAttackTargeting;
		Widget->Feedback = FText::GetEmpty();
		if (APlayerController* PC = Cast<APlayerController>(Player->GetController()))
		{
			// Destination selection uses the visible pointer; keep camera input blocked.
			PC->bShowMouseCursor = true;
		}
		return;
	}
	if (Choice == ERCompanionMenuChoice::Follow && !Player->CompanionParty->Follow())
	{
		UE_LOG(LogTemp, Warning, TEXT("Companion Follow rejected: %s has controller %s; expected AAIC_Companion."),
			*GetNameSafe(Player->CompanionParty->SelectedCompanion),
			*GetNameSafe(IsValid(Player->CompanionParty->SelectedCompanion) ? Player->CompanionParty->SelectedCompanion->GetController() : nullptr));
	}
	else if (Choice == ERCompanionMenuChoice::Stay) Player->CompanionParty->Stay();
	Cancel();
}

void URCompanionCommandComponent::Cancel()
{
	// Possession may have ended before EndPlay; restore the controller we blocked.
	APlayerController* PC = MenuController.Get();
	if (PC)
	{
		if (bMovementBlocked) PC->SetIgnoreMoveInput(false);
		if (bLookBlocked) PC->SetIgnoreLookInput(false);
		if (IsCommandInputActive()) PC->bShowMouseCursor = bSavedCursor;
	}
	bMenuOpen = bTargeting = bAttackTargeting = bMovementBlocked = bLookBlocked = false;
	SetComponentTickEnabled(false);
	if (IsValid(MarkerRecipient) && MarkerRecipient->SelectionMarker) MarkerRecipient->SelectionMarker->SetVisibility(false);
	MarkerRecipient = nullptr;
	TargetRecipient = nullptr;
	if (InputSubsystem.IsValid() && ActiveContext) InputSubsystem->RemoveMappingContext(ActiveContext);
	if (Widget) Widget->RemoveFromParent();
	Widget = nullptr;
	MenuController.Reset();
}

void URCompanionCommandComponent::ConfirmTarget()
{
	if (!bTargeting || !Widget) return;
	ARPlayer* Player = Cast<ARPlayer>(GetOwner());
	APlayerController* PC = Player ? Cast<APlayerController>(Player->GetController()) : nullptr;
	if (!PC || !IsValid(TargetRecipient) || !Player->CompanionParty->Companions.Contains(TargetRecipient)) { Cancel(); return; }
	FVector Start;
	FVector Direction;
	if (!PC->DeprojectMousePositionToWorld(Start, Direction))
	{
		Widget->Feedback = NSLOCTEXT("CompanionUI", "NoPointer", "Move the pointer over the game viewport.");
		return;
	}
	FHitResult Hit;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(CompanionDestination), false, Player);
	for (ARCompanion* Companion : Player->CompanionParty->Companions) Params.AddIgnoredActor(Companion);
	if (bAttackTargeting)
	{
		AAIC_Companion* AI = Cast<AAIC_Companion>(TargetRecipient->GetController());
		if (AI && GetWorld()->LineTraceSingleByChannel(Hit, Start, Start + Direction * TargetRange, ECC_Visibility, Params)
			&& AI->Attack(Hit.GetActor())) Cancel();
		else Widget->Feedback = NSLOCTEXT("CompanionUI", "InvalidEnemy", "Choose a living enemy. Check the companion's combat setup if the order is rejected.");
		return;
	}
	UNavigationSystemV1* Navigation = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	FNavLocation Destination;
	AAIC_Companion* AI = Cast<AAIC_Companion>(TargetRecipient->GetController());
	if (AI && Navigation && GetWorld()->LineTraceSingleByChannel(Hit, Start, Start + Direction * TargetRange, ECC_Visibility, Params) && Hit.ImpactNormal.Z >= 0.5f &&
		Navigation->ProjectPointToNavigation(Hit.ImpactPoint, Destination, FVector(100.f, 100.f, 150.f)) && AI->GoToLocation(Destination.Location))
	{
		Cancel();
	}
	else Widget->Feedback = NSLOCTEXT("CompanionUI", "Invalid", "Destination unavailable. Aim elsewhere or press Esc.");
}

void URCompanionCommandComponent::CycleCompanion(int32 Direction)
{
	ARPlayer* Player = Cast<ARPlayer>(GetOwner());
	if (!Player || !Player->CompanionParty) return;
	URCompanionPartyComponent* Party = Player->CompanionParty;
	const int32 Count = Party->Companions.Num();
	int32 Index = Party->Companions.IndexOfByKey(Party->SelectedCompanion);
	if (Index == INDEX_NONE) Index = Direction > 0 ? Count - 1 : 0;
	for (int32 Attempt = 0; Attempt < Count; ++Attempt)
	{
		Index = (Index + Direction + Count) % Count;
		if (Party->SelectCompanion(Party->Companions[Index])) { UpdateLabel(); return; }
	}
}

void URCompanionCommandComponent::NextCompanion() { if (bMenuOpen) CycleCompanion(1); }
void URCompanionCommandComponent::PreviousCompanion() { if (bMenuOpen) CycleCompanion(-1); }

void URCompanionCommandComponent::UpdateLabel()
{
	ARPlayer* Player = Cast<ARPlayer>(GetOwner());
	ARCompanion* Companion = Player && Player->CompanionParty ? Player->CompanionParty->SelectedCompanion.Get() : nullptr;
	if (!Widget || !IsValid(Companion)) return;
	AAIC_Companion* AI = Cast<AAIC_Companion>(Companion->GetController());
	const FString State = AI ? StaticEnum<ERCompanionOrder>()->GetDisplayNameTextByValue(static_cast<int64>(AI->CurrentOrder)).ToString() : TEXT("No controller");
	const FText Name = Companion->CompanionDisplayName.IsEmpty() ? FText::FromString(Companion->GetName()) : Companion->CompanionDisplayName;
	Widget->CompanionLabel = FText::Format(NSLOCTEXT("CompanionUI", "NameState", "{0} — {1}"), Name, FText::FromString(State));
	if (MarkerRecipient != Companion)
	{
		if (IsValid(MarkerRecipient) && MarkerRecipient->SelectionMarker) MarkerRecipient->SelectionMarker->SetVisibility(false);
		MarkerRecipient = Companion;
	}
	if (Companion->SelectionMarker) Companion->SelectionMarker->SetVisibility(true);
}

void URCompanionCommandComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	ARPlayer* Player = Cast<ARPlayer>(GetOwner());
	APlayerController* PC = Player ? Cast<APlayerController>(Player->GetController()) : nullptr;
	if (!PC || PC != MenuController.Get() || !IsValid(MarkerRecipient)) { Cancel(); return; }
	if (MarkerRecipient->SelectionMarker)
	{
		FVector CameraLocation;
		FRotator CameraRotation;
		PC->GetPlayerViewPoint(CameraLocation, CameraRotation);
		MarkerRecipient->SelectionMarker->SetWorldRotation((CameraLocation - MarkerRecipient->SelectionMarker->GetComponentLocation()).Rotation());
	}
}

void URCompanionCommandComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Cancel();
	if (InputSubsystem.IsValid() && OpenContext) InputSubsystem->RemoveMappingContext(OpenContext);
	if (BoundInput.IsValid()) for (uint32 Handle : BindingHandles) BoundInput->RemoveBindingByHandle(Handle);
	Super::EndPlay(EndPlayReason);
}
