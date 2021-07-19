// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MenuInterface.h"
#include "PregameMenu.generated.h"

class UButton;
class UEditableTextBox;

/**
 * 
 */
UCLASS()
class RTSPROTOTYPE_API UPregameMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetMenuInterface(IMenuInterface* MenuInterfaceBase);

	void Setup();
	void Teardown();

protected:
	virtual bool Initialize();

private:
	
	UPROPERTY(meta = (BindWidget))
	UButton* ConfirmUsernameButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ReadyButton;

	UPROPERTY(meta = (BindWidget))
	UButton* QuitSessionButton;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* UsernameField;

	UFUNCTION()
	void SetUsername();

	UFUNCTION()
	void PlayerReady();

	UFUNCTION()
	void Quit();

	IMenuInterface* MenuInterface;
};
