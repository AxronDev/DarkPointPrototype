// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "UnitAIController.generated.h"

class ARTSPrototypeCharacter;
class UAISenseConfig_Sight;
class UAISense;
class UAIPerceptionComponent;
struct FTimerHandle;
class IPlaceable;

/**
 * 
 */
UCLASS()
class RTSPROTOTYPE_API AUnitAIController : public AAIController
{
	GENERATED_BODY()
	
	AUnitAIController();

public:
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void MoveUnit(FVector Destination);

    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const override;
    
    UFUNCTION(Server, Reliable, WithValidation)
    void Server_GetAICharacter();

    void SetPatroling(bool NewPatrol);

    FHitResult HitLoc;

    UPROPERTY(replicated)
    TArray<FHitResult> QueuedMovements{};

    UFUNCTION(Server, Reliable, WithValidation)
	void Server_ResetMoveQueue();

    void PreResetQueue();

	// Place in queue
	uint8 QueueNum = 0;

    // UFUNCITON(Server, Reliable, WithValidation)
    void SetHit(FHitResult& InHit, bool Aggressive);

    UPROPERTY(EditAnywhere)
    TSubclassOf<UAISense> SightSenseClass;

    UPROPERTY(EditAnywhere)
    TSubclassOf<UAISense> NoneSenseClass;

    TArray<IPlaceable*> EnemyUnits;
    // TArray<AActor*>& Actors;
    
    IPlaceable* Target;
    
protected:
    virtual void BeginPlay() override;

    UAISenseConfig_Sight* SightSenseConfig;

    UPROPERTY(EditDefaultsOnly)
    UAIPerceptionComponent* AIPercep;

private:

    UFUNCTION(Server, Reliable, WithValidation)
    void Server_MoveTo(FHitResult Hit, bool Aggressive);

    FTimerHandle Timer;

    void NextMoveQueue(bool Aggro);

    UPROPERTY(replicated)
    bool bPatroling;

    bool WithinRange();

    void CanAttack();

    UPROPERTY(replicated)
    ARTSPrototypeCharacter* Char;

    void AssignTarget();

    float MaxAge = 5.f;

    UFUNCTION()
    void SortEnemyObjects(const TArray<AActor*>& Actors);
};
