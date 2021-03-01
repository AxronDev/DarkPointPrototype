// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Building.generated.h"

class UDecalComponent;
class USceneComponent;
class UAIPerceptionStimuliSourceComponent;
class UAISense;

UCLASS()
class RTSPROTOTYPE_API ABuilding : public AActor
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	ABuilding();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite)
	FName BuildingType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxHealth = 500;

	UPROPERTY(EditDefaultsOnly, replicated)
	float Health = MaxHealth;

	UPROPERTY(EditDefaultsOnly)
	UAIPerceptionStimuliSourceComponent* Stimuli;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAISense> SightSense;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const override;

    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, class AController * EventInstigator, AActor * DamageCauser) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDecalComponent* CursorToWorld;

	UPROPERTY(EditDefaultsOnly)
	USceneComponent* Root;

	FName GetBuildingType();

	void SetOwnerUserName(FName UserName);

	UFUNCTION(BlueprintCallable)
	FName GetOwnerUserName();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<bool> AttackSlots;

	UPROPERTY(replicated)
	bool bHasBeenPositioned = false;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Decal")
	void CanPlace(bool val);

	UPROPERTY(BlueprintReadWrite)
	bool bCanPlace = false;

	UFUNCTION(BlueprintCallable)
	float GetHealth();

private:
	UPROPERTY(replicated)
	FName OwnerUserName = "";

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Death();

};
