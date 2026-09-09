// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/AsyncTasks/RAbilityTask_OnTick.h"

URAbilityTask_OnTick::URAbilityTask_OnTick()
{
	bTickingTask = true;
}

URAbilityTask_OnTick* URAbilityTask_OnTick::ExecuteTaskOnTick(UGameplayAbility* OwningAbility)
{
	URAbilityTask_OnTick* TaskNode = NewAbilityTask<URAbilityTask_OnTick>(OwningAbility);

	return TaskNode;
}

void URAbilityTask_OnTick::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnAbilityTaskTick.Broadcast(DeltaTime);
	}
	else
	{
		EndTask();
	}
}
