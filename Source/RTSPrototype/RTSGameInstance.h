// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MenuSystem/MenuInterface.h"
#include "RTSGameInstance.generated.h"

class UUserWidget;
class UMainMenu;
class UPregameMenu;
class UPauseMenu;
class ARtsPlayerController;

/**
 * 
 */
UCLASS()
class RTSPROTOTYPE_API URTSGameInstance : public UGameInstance, public IMenuInterface
{
	GENERATED_BODY()
	
	URTSGameInstance(const FObjectInitializer& ObjectInitializer);

	virtual void Init();
	
	UFUNCTION(BlueprintCallable)
	void LoadMenu();

	UFUNCTION(BlueprintCallable)
	void LoadPregameMenu();

	UFUNCTION()
	void Host();

	UFUNCTION()
	void Join(const FString& Address);

	UFUNCTION()
	void QuitSession();

	UFUNCTION()
	void SetUsername(const FName& NewUsernameToSet);

private:
	TSubclassOf<UUserWidget> MenuClass;
	TSubclassOf<UUserWidget> PauseMenuClass;
	TSubclassOf<UUserWidget> PregameMenuClass;
	
	uint8 NumPlayers{0};

	UMainMenu* Menu;
	UPregameMenu* PregameMenu;
	UPauseMenu* PauseMenu;

public:
	UFUNCTION(BlueprintCallable)
	virtual void LoadPauseMenu();

	void NewPlayerController(ARtsPlayerController* NewPlayerController);

	virtual void SetPlayerReady(ARtsPlayerController* ReadyPlayer);
};
