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

    UPROPERTY(EditAnywhere)
    TSubclassOf<UAISense> SightSenseClass;

    UPROPERTY(EditAnywhere)
    TSubclassOf<UAISense> NoneSenseClass;

    TArray<ARTSPrototypeCharacter*> EnemyUnits;
    // TArray<AActor*>& Actors;
    
    ARTSPrototypeCharacter* Target;
    
protected:
    virtual void BeginPlay() override;

    UAISenseConfig_Sight* SightSenseConfig;

    UAIPerceptionComponent* AIPercep;

private:

    FTimerHandle Timer;

    bool WithinRange();

    void CanAttack();

    UPROPERTY(replicated)
    ARTSPrototypeCharacter* Char;

    void AssignTarget();

    float MaxAge = 5.f;

    UFUNCTION()
    void SortEnemyObjects(const TArray<AActor*>& Actors);
};
