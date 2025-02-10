// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/RSurvivalGameMode.h"

ARSurvivalGameMode::ARSurvivalGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void ARSurvivalGameMode::BeginPlay()
{
	Super::BeginPlay();
	checkf(EnemyWaveSpawnerDataTable,TEXT("Forgot to assign a valid data table in survival game mode blueprint"));
	SetCurrentSurvivalState(ESurvivalState::WaitSpawnNewWave);
	TotalWavesToSpawn = EnemyWaveSpawnerDataTable->GetRowNames().Num();
}

void ARSurvivalGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentSurvivalState == ESurvivalState::WaitSpawnNewWave)
	{
		TimePassedSinceStart += DeltaTime;
		if (TimePassedSinceStart >= SpawnNewWaveWaitTime)
		{
			TimePassedSinceStart = 0.f;
			SetCurrentSurvivalState(ESurvivalState::SpawningNewWave);
		}
	}
	if (CurrentSurvivalState == ESurvivalState::SpawningNewWave)
	{
		TimePassedSinceStart += DeltaTime;
		if (TimePassedSinceStart >= SpawnEnemiesDelayTime)
		{
			//TODO:Handle spawn new enemies
			TimePassedSinceStart = 0.f;
			SetCurrentSurvivalState(ESurvivalState::InProgress);
		}
	}
	if (CurrentSurvivalState == ESurvivalState::WaveCompleted)
	{
		TimePassedSinceStart += DeltaTime;
		if (TimePassedSinceStart >= WaveCompletedWaitTime)
		{
			TimePassedSinceStart = 0.f;
			CurrentWaveCount++;
			if (HasFinishedAllWaves())
			{
				SetCurrentSurvivalState(ESurvivalState::AllWavesDone);
			}
			else
			{
				SetCurrentSurvivalState(ESurvivalState::WaitSpawnNewWave);
			}
		}
	}
}

void ARSurvivalGameMode::SetCurrentSurvivalState(ESurvivalState InState)
{
	CurrentSurvivalState = InState;
	OnSurvivalStateChanged.Broadcast(CurrentSurvivalState);
}
bool ARSurvivalGameMode::HasFinishedAllWaves() const
{
	return CurrentWaveCount>TotalWavesToSpawn;
}