// Fill out your copyright notice in the Description page of Project Settings.


#include "PauseMenu.h"
#include "Components/Button.h"

bool UPauseMenu::Initialize() 
{
     bool Success = Super::Initialize();
     if(!Success) return false;

     if(!ensure(BackButton)) return false;
     BackButton->OnClicked.AddDynamic(this, &UPauseMenu::Teardown);

     if(!ensure(QuitSessionButton)) return false;
     QuitSessionButton->OnClicked.AddDynamic(this, &UPauseMenu::QuitSession);
     return true;
}

void UPauseMenu::QuitSession() 
{
     if(MenuInterface)
     {
          MenuInterface->QuitSession();
     }
}

void UPauseMenu::SetMenuInterface(IMenuInterface* MenuInterfaceBase) 
{
     MenuInterface = MenuInterfaceBase;
}

void UPauseMenu::Setup() 
{
     this->AddToViewport();

     UWorld* World = GetWorld();
     if(!ensure(World)) return;

     APlayerController* PlayerController = World->GetFirstPlayerController();
     if(!ensure(PlayerController)) return;

     this->bIsFocusable = true;
     FInputModeUIOnly InputModeData;
     InputModeData.SetWidgetToFocus(this->TakeWidget());
     InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

     PlayerController->SetInputMode(InputModeData);

     PlayerController->bShowMouseCursor = true;
}

void UPauseMenu::Teardown() 
{
     this->RemoveFromViewport();

     UWorld* World = GetWorld();
     if(!ensure(World)) return;

     APlayerController* PlayerController = World->GetFirstPlayerController();
     if(!ensure(PlayerController)) return;

     FInputModeGameOnly InputModeData;
     PlayerController->SetInputMode(InputModeData);

     PlayerController->bShowMouseCursor = false;
}
