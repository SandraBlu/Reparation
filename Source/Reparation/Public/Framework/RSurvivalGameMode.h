// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/RGameMode.h"
#include "RSurvivalGameMode.generated.h"

class AREnemy;

UENUM(BlueprintType)
enum class ESurvivalState: uint8
{
	WaitSpawnNewWave,
	SpawningNewWave,
	InProgress,
	WaveCompleted,
	AllWavesDone,
	PlayerDied
};

USTRUCT(BlueprintType)
struct FEnemyWaveSpawnerInfo
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere)
	TSoftClassPtr<AREnemy> SoftEnemyClassToSpawn;
	UPROPERTY(EditAnywhere)
	int32 MinPerSpawnCount = 1;
	UPROPERTY(EditAnywhere)
	int32 MaxPerSpawnCount = 3;
};
USTRUCT(BlueprintType)
struct FEnemyWaveSpawnerTableRow : public FTableRowBase
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere)
	TArray<FEnemyWaveSpawnerInfo> WaveSpawnerDefinitions;
	UPROPERTY(EditAnywhere)
	int32 TotalEnemyToSpawnThisWave = 1;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSurvivalStateChangedDelegate,ESurvivalState,CurrentState);
/**
 * 
 */
UCLASS()
class REPARATION_API ARSurvivalGameMode : public ARGameMode
{
	GENERATED_BODY()

public:
	ARSurvivalGameMode();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
private:
	void SetCurrentSurvivalState(ESurvivalState InState);

	bool HasFinishedAllWaves() const;
	
	UPROPERTY()
	ESurvivalState CurrentSurvivalState;
	
	UPROPERTY(BlueprintAssignable,BlueprintCallable)
	FOnSurvivalStateChangedDelegate OnSurvivalStateChanged;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WaveDefinition", meta = (AllowPrivateAccess = "true"))
	UDataTable* EnemyWaveSpawnerDataTable;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "WaveDefinition", meta = (AllowPrivateAccess = "true"))
	int32 TotalWavesToSpawn;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "WaveDefinition", meta = (AllowPrivateAccess = "true"))
	int32 CurrentWaveCount = 1;
	
	UPROPERTY()
	float TimePassedSinceStart = 0.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WaveDefinition", meta = (AllowPrivateAccess = "true"))
	float SpawnNewWaveWaitTime = 5.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WaveDefinition", meta = (AllowPrivateAccess = "true"))
	float SpawnEnemiesDelayTime = 2.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WaveDefinition", meta = (AllowPrivateAccess = "true"))
	float WaveCompletedWaitTime = 5.f;
};
