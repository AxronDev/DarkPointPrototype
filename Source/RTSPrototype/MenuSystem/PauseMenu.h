// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MenuInterface.h"
#include "PauseMenu.generated.h"

class UButton;
class UEditableTextBox;

/**
 * 
 */
UCLASS()
class RTSPROTOTYPE_API UPauseMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetMenuInterface(IMenuInterface* MenuInterfaceBase);

	UFUNCTION()
	void Setup();

	UFUNCTION()
	void Teardown();

protected:
	bool Initialize();

private:
	IMenuInterface* MenuInterface;

	UPROPERTY(meta = (BindWidget))
	UButton* BackButton;

	UPROPERTY(meta = (BindWidget))
	UButton* QuitSessionButton;

	UFUNCTION()
	void QuitSession();
};
