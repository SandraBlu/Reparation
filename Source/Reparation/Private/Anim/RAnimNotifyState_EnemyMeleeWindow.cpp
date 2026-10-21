#include "Anim/RAnimNotifyState_EnemyMeleeWindow.h"
#include "Components/Combat/REnemyMeleeSweepComponent.h"
#include "Components/SkeletalMeshComponent.h"

static UREnemyMeleeSweepComponent* GetSweep(USkeletalMeshComponent* Mesh)
{
	return Mesh && Mesh->GetOwner() ? Mesh->GetOwner()->FindComponentByClass<UREnemyMeleeSweepComponent>() : nullptr;
}

void URAnimNotifyState_EnemyMeleeWindow::NotifyBegin(USkeletalMeshComponent* Mesh, UAnimSequenceBase* Animation, float Duration, const FAnimNotifyEventReference& Reference)
{
	Super::NotifyBegin(Mesh, Animation, Duration, Reference);
	if (UREnemyMeleeSweepComponent* Sweep = GetSweep(Mesh)) Sweep->BeginWindow(DamageEventTag);
}

void URAnimNotifyState_EnemyMeleeWindow::NotifyTick(USkeletalMeshComponent* Mesh, UAnimSequenceBase* Animation, float DeltaTime, const FAnimNotifyEventReference& Reference)
{
	Super::NotifyTick(Mesh, Animation, DeltaTime, Reference);
	if (UREnemyMeleeSweepComponent* Sweep = GetSweep(Mesh)) Sweep->SampleWindow();
}

void URAnimNotifyState_EnemyMeleeWindow::NotifyEnd(USkeletalMeshComponent* Mesh, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& Reference)
{
	if (UREnemyMeleeSweepComponent* Sweep = GetSweep(Mesh)) Sweep->EndWindow();
	Super::NotifyEnd(Mesh, Animation, Reference);
}
