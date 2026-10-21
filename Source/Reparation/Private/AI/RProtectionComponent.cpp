#include "AI/RProtectionComponent.h"
#include "AI/RAIController.h"
#include "Characters/RCharacterBase.h"
#include "Stealth/RStealthComponent.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "UI/GAS/RProtectionWarningWidget.h"
#include "TimerManager.h"
URProtectionComponent::URProtectionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.25f;
    SetIsReplicatedByDefault(true);
}
void URProtectionComponent::BeginPlay() { Super::BeginPlay(); ProtectionCenter = GetOwner()->GetActorLocation(); }
void URProtectionComponent::SetProtectionCenter(FVector Center) { ProtectionCenter = Center; WarningStarts.Empty(); Provoked.Empty(); }
bool URProtectionComponent::IsVisibleIntruder(AActor* Actor) const
{
    const APawn* OwnerPawn = Cast<APawn>(GetOwner());
    const AAIController* AI = OwnerPawn ? Cast<AAIController>(OwnerPawn->GetController()) : nullptr;
    if (!AI || !IsValid(Actor) || Actor == GetOwner() || FVector::DistSquared(ProtectionCenter, Actor->GetActorLocation()) > FMath::Square(ProtectionRadius)) return false;
    if (AI->GetTeamAttitudeTowards(*Actor) != ETeamAttitude::Hostile) return false;
    if (const ARCharacterBase* Character = Cast<ARCharacterBase>(Actor); Character && (Character->IsDead_Implementation() || Character->Stealth->IsConcealed())) return false;
    if (const ARAIController* EnemyAI = Cast<ARAIController>(AI); EnemyAI && !EnemyAI->IsWithinSightCone(Actor->GetActorLocation())) return false;
    return AI->LineOfSightTo(Actor);
}
bool URProtectionComponent::CanEngage(AActor* Actor) const
{
    if (WorldRole != ERWorldRole::Guard && WorldRole != ERWorldRole::Sentry) return true;
    if (Provoked.Contains(Actor)) return true;
    if (!IsValid(Actor) || FVector::DistSquared(ProtectionCenter, Actor->GetActorLocation()) > FMath::Square(ProtectionRadius)) return false;
    if (!bWarnBeforeAttack || WarningGraceSeconds <= 0.f) return true;
    const double* Start = WarningStarts.Find(Actor);
    return Start && GetWorld()->GetTimeSeconds() - *Start >= WarningGraceSeconds;
}
void URProtectionComponent::NotifyProvoked(AActor* Actor) { if (IsValid(Actor)) Provoked.Add(Actor); }
void URProtectionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!GetOwner()->HasAuthority() || (WorldRole != ERWorldRole::Guard && WorldRole != ERWorldRole::Sentry)) return;
    if (const ARCharacterBase* Character = Cast<ARCharacterBase>(GetOwner()); Character && Character->IsDead_Implementation()) return;
    for (auto It = WarningStarts.CreateIterator(); It; ++It) if (!IsVisibleIntruder(It.Key().Get())) It.RemoveCurrent();
    for (TActorIterator<APawn> It(GetWorld()); It; ++It)
    {
        AActor* Actor = *It;
        if (!IsVisibleIntruder(Actor) || WarningStarts.Contains(Actor)) continue;
        WarningStarts.Add(Actor, GetWorld()->GetTimeSeconds());
        if (bWarnBeforeAttack && !Provoked.Contains(Actor)) MulticastWarning(Actor, WarningText);
    }
}
void URProtectionComponent::MulticastWarning_Implementation(AActor* Intruder, const FText& Text)
{
    OnProtectionWarning.Broadcast(Intruder, Text);
    APawn* Pawn = Cast<APawn>(Intruder);
    APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
    if (!PC || !PC->IsLocalController()) return;
    URProtectionWarningWidget* Widget = CreateWidget<URProtectionWarningWidget>(PC);
    if (!Widget) return;
    Widget->Warning = Text;
    Widget->AddToViewport(50);
    Widget->SetDesiredSizeInViewport(FVector2D(650.f, 60.f));
    Widget->SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
    int32 Width, Height;
    PC->GetViewportSize(Width, Height);
    Widget->SetPositionInViewport(FVector2D(Width * 0.5f, Height * 0.2f), false);
    FTimerHandle Timer;
    GetWorld()->GetTimerManager().SetTimer(Timer, FTimerDelegate::CreateWeakLambda(Widget, [Widget]() { Widget->RemoveFromParent(); }), 3.f, false);
}
