// Fill out your copyright notice in the Description page of Project Settings.

#include "APBuffItem.h"
#include "ProjectOne/Player/ProjectOneCharacter.h"
#include "Player/POComponents/PlayerStatComponent.h"
#include "Player/POComponents/POPlayerAkComponent.h"

AAPBuffItem::AAPBuffItem() {
	eItem = E_Item::E_AP_BUFF;
	static ConstructorHelpers::FObjectFinder<UParticleSystem> P_Shield(TEXT("ParticleSystem'/Game/rgyEffect01/Particle/P07/P07_Shield.P07_Shield'"));
	if (P_Shield.Succeeded())
	{
		UseEffect = P_Shield.Object;
	}
}

void AAPBuffItem::Use() {
	ItemOwner->SheildEffect->SetTemplate(UseEffect);
	ItemOwner->SoundManager->SetAkSwitch(TEXT("Item"), TEXT("Item_AP_Buff"));
	ItemOwner->SoundManager->PlayItemSound();
	ItemOwner->SheildEffect->SetActive(true);
	ItemOwner->CharacterStat->CurAP += 5;
}

void AAPBuffItem::DurationEnd()
{
	ItemOwner->CharacterStat->CurAP -= 5;
}
