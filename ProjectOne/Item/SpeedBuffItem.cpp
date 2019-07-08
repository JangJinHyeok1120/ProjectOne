// Fill out your copyright notice in the Description page of Project Settings.

#include "SpeedBuffItem.h"
#include "ProjectOne/Player/ProjectOneCharacter.h"
#include "Player/POComponents/POPlayerAkComponent.h"
ASpeedBuffItem::ASpeedBuffItem() {
	eItem = E_Item::E_SPEED_BUFF;
}

void ASpeedBuffItem::Use()
{
	ItemOwner->GetCharacterMovement()->MaxWalkSpeed = 700.0f;
	DurationTime = 5.0f;
	GetWorld()->GetTimerManager().SetTimer(DurationTimer, this, &ASpeedBuffItem::Duration, 1.0f, true);

	ItemOwner->SoundManager->SetAkSwitch(TEXT("Item"), TEXT("Item_Speed_Buff"));
	ItemOwner->SoundManager->PlayItemSound();
}

void ASpeedBuffItem::DurationEnd()
{
	if(ItemOwner)
		ItemOwner->GetCharacterMovement()->MaxWalkSpeed = 600.0f;
}
