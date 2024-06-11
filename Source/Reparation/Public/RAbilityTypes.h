#pragma once

#include "GameplayEffectTypes.h"
#include "RAbilityTypes.generated.h"

class UGameplayEffect;

USTRUCT(BlueprintType)
struct FRGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

public:
	
	bool IsCriticalHit() const { return bIsCriticalHit; }
	bool IsBlockedHit () const { return bIsBlockedHit; }
	bool IsDodgedHit() const { return bIsDodgedHit; }
	//TSharedPtr<FGameplayTag> GetDamageType() const { return DamageType; }

	void SetIsCriticalHit(bool bInIsCriticalHit) { bIsCriticalHit = bInIsCriticalHit; }
	void SetIsBlockedHit(bool bInIsBlockedHit) { bIsBlockedHit = bInIsBlockedHit; }
	void SetIsDodgedHit(bool bInIsDodgedHit) { bIsDodgedHit = bInIsDodgedHit; }
	//void SetDamageType(TSharedPtr<FGameplayTag> InDamageType) { DamageType = InDamageType; }

	/** Returns the actual struct used for serialization, subclasses must override this! */
	virtual UScriptStruct* GetScriptStruct() const
	{
		return StaticStruct();
	}
	/** Creates a copy of this context, used to duplicate for later modifications */
	virtual FRGameplayEffectContext* Duplicate() const
	{
		FRGameplayEffectContext* NewContext = new FRGameplayEffectContext();
		*NewContext = *this;
		if (GetHitResult())
		{
			// Does a deep copy of the hit result
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}
	/** Custom serialization, subclasses must override this */
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);

protected:

	UPROPERTY()
	bool bIsBlockedHit = false;

	UPROPERTY()
	bool bIsCriticalHit = false;

	UPROPERTY()
	bool bIsDodgedHit = false;
	
	TSharedPtr<FGameplayTag> DamageType;
};

template<>
struct TStructOpsTypeTraits<FRGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FRGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};