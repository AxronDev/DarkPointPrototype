// Fill out your copyright notice in the Description page of Project Settings.


#include "PregameMenu.h"
#include "Components/Button.h"
#include "RTSPrototype\RtsPlayerController.h"
#include "Components/EditableTextBox.h"

bool UPregameMenu::Initialize() 
{
     bool Success = Super::Initialize();
     if(!Success) return false;

     if(!ensure(ConfirmUsernameButton)) return false;
     ConfirmUsernameButton->OnClicked.AddDynamic(this, &UPregameMenu::SetUsername);

     if(!ensure(ReadyButton)) return false;
     ReadyButton->OnClicked.AddDynamic(this, &UPregameMenu::PlayerReady);

     if(!ensure(QuitSessionButton)) return false;
     QuitSessionButton->OnClicked.AddDynamic(this, &UPregameMenu::Quit);

     return true;
}

void UPregameMenu::SetUsername() 
{
     if(!ensure(UsernameField))
     {
          UE_LOG(LogTemp, Warning, TEXT("SetUsername failed"));
          return;
     }
     if(MenuInterface)
     {
          const FName& NewUsername = FName(UsernameField->GetText().ToString());
          MenuInterface->SetUsername(NewUsername);
     }
}

void UPregameMenu::PlayerReady() 
{
     MenuInterface->SetPlayerReady(Cast<ARtsPlayerController>(GetWorld()->GetFirstPlayerController()));
}

void UPregameMenu::Quit() 
{
     UWorld* World = GetWorld();
     if(ensure(!World)) return;

     APlayerController* PlayerController = World->GetFirstPlayerController();
     if(!ensure(PlayerController)) return;

     PlayerController->ConsoleCommand("Quit", false);
}

void UPregameMenu::SetMenuInterface(IMenuInterface* MenuInterfaceBase) 
{
     MenuInterface = MenuInterfaceBase;
}

void UPregameMenu::Setup() 
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

void UPregameMenu::Teardown() 
{
     this->RemoveFromViewport();
}
