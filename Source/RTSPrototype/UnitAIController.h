// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AISenseConfig_Sight.h"
#include "UnitAIController.generated.h"

class ARTSPrototypeCharacter;
class UAISenseConfig_Sight;
class UAISense;
class UAIPerceptionComponent;

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

	UPROPERTY(EditAnywhere)
	TSubclassOf<UAISense> SightSenseClass;
	
protected:
	virtual void BeginPlay() override;

	UAISenseConfig_Sight* SightSenseConfig;

	UAIPerceptionComponent* AIPercep;

private:
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_GetAICharacter();

	UPROPERTY(replicated)
	ARTSPrototypeCharacter* Char;
};
