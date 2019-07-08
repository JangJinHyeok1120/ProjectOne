// Fill out your copyright notice in the Description page of Project Settings.

#include "POPlayerAkComponent.h"
#include "AkAudioEvent.h"
#include "AkAudioDevice.h"


UPOPlayerAkComponent::UPOPlayerAkComponent() {
	ConstructorHelpers::FObjectFinder<UAkAudioEvent> PISTOLSOUND(TEXT("AkAudioEvent'/Game/Sound/Character/Weapon_Shot.Weapon_Shot'"));
	if (PISTOLSOUND.Succeeded()) {
		PistolSound = PISTOLSOUND.Object;
	}
	ConstructorHelpers::FObjectFinder<UAkAudioEvent> HITSOUND(TEXT("AkAudioEvent'/Game/Sound/Character/Hit.Hit'"));
	if (HITSOUND.Succeeded()) {
		BulletCrashSound = HITSOUND.Object;
	}
	ConstructorHelpers::FObjectFinder<UAkAudioEvent> ELECTRICSOUND(TEXT("AkAudioEvent'/Game/Sound/Character/Electric_Field.Electric_Field'"));
	if (ELECTRICSOUND.Succeeded()) {
		ElectricSound = ELECTRICSOUND.Object;
	}
	ConstructorHelpers::FObjectFinder<UAkAudioEvent> EVOLSOUND(TEXT("AkAudioEvent'/Game/Sound/Character/Evolution.Evolution'"));
	if (EVOLSOUND.Succeeded()) {
		EvolutionSound = EVOLSOUND.Object;
	}
	ConstructorHelpers::FObjectFinder<UAkAudioEvent> ITEMSOUND(TEXT("AkAudioEvent'/Game/Sound/Character/Item.Item'"));
	if (ITEMSOUND.Succeeded()) {
		ItemSound = ITEMSOUND.Object;
	}
	ConstructorHelpers::FObjectFinder<UAkAudioEvent> ITEMGETSOUND(TEXT("AkAudioEvent'/Game/Sound/Character/Item_Gain.Item_Gain'"));
	if (ITEMGETSOUND.Succeeded()) {
		ItemGetSound = ITEMGETSOUND.Object;
	}
}

void UPOPlayerAkComponent::PlayPistolShotSound()
{
	FAkAudioDevice::Get()->PostEvent(PistolSound, GetOwner());
}


void UPOPlayerAkComponent::PlayBulletCrashSound()
{
	FAkAudioDevice::Get()->PostEvent(BulletCrashSound, GetOwner());
}

void UPOPlayerAkComponent::PlayEvolutionSound()
{
	FAkAudioDevice::Get()->PostEvent(EvolutionSound, GetOwner());
}

void UPOPlayerAkComponent::PlayItemSound()
{
	FAkAudioDevice::Get()->PostEvent(ItemSound, GetOwner());
}

void UPOPlayerAkComponent::PlayItemGetSound()
{
	FAkAudioDevice::Get()->PostEvent(ItemGetSound, GetOwner());
}

void UPOPlayerAkComponent::PlayElectricFieldIn()
{
	FAkAudioDevice::Get()->SetSwitch(TEXT("Electric_Field"), TEXT("In"), GetOwner());
	FAkAudioDevice::Get()->PostEvent(ElectricSound, GetOwner());
}

void UPOPlayerAkComponent::PlayElectricFieldOut()
{
	FAkAudioDevice::Get()->SetSwitch(TEXT("Electric_Field"), TEXT("Out"), GetOwner());
	FAkAudioDevice::Get()->PostEvent(ElectricSound, GetOwner());
}

void UPOPlayerAkComponent::SetAkSwitch(const TCHAR* group, const TCHAR* name)
{
	FAkAudioDevice::Get()->SetSwitch(group, name, GetOwner());
}
