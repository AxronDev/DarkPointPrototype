// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MenuInterface.h"
#include "MainMenu.generated.h"

class UButton;
class UWidgetSwitcher;
class UCanvasPanel;
class UVerticalBox;
class UEditableTextBox;

/**
 * 
 */
UCLASS()
class RTSPROTOTYPE_API UMainMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetMenuInterface(IMenuInterface* MenuInterfaceBase);

	void Setup();
	void Teardown();

protected:
	virtual bool Initialize();

private:

	/* UPROPERTY(meta = (BindWidget))
	UButton* EnterUserNameButton; */

	UPROPERTY(meta = (BindWidget))
	UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinIPButton;

	UPROPERTY(meta = (BindWidget))
	UButton* BackButton;

	UPROPERTY(meta = (BindWidget))
	UButton* QuitButton;

	UPROPERTY(meta = (BindWidget))
	UWidgetSwitcher* MenuSwitcher;

	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* MainMenu;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* JoinMenu; 

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* IPAddressField;
	
	/* UPROPERTY(meta = (BindWidget))
	UEditableTextBox* UserNameEntry; */

	/* UFUNCTION()
	void SetUsername(); */

	UFUNCTION()
	void HostServer();

	UFUNCTION()
	void JoinIP();

	UFUNCTION()
	void OpenJoinMenu();

	UFUNCTION()
	void OpenMainMenu();

	UFUNCTION()
	void Quit();

	IMenuInterface* MenuInterface;
};
