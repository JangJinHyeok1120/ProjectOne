// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ProjectOne.h"
#include "AkComponent.h"
#include "POPlayerAkComponent.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTONE_API UPOPlayerAkComponent : public UAkComponent
{
	GENERATED_BODY()
public:
	UPOPlayerAkComponent();
	void PlayPistolShotSound();
	void PlayBulletCrashSound();
	void PlayEvolutionSound();
	void PlayItemSound();
	void PlayItemGetSound();

	UFUNCTION(BlueprintCallable)
	void PlayElectricFieldIn();

	UFUNCTION(BlueprintCallable)
	void PlayElectricFieldOut();
	
	void SetAkSwitch(const TCHAR* group, const TCHAR* name);
public:
	UAkAudioEvent * PistolSound;
	UAkAudioEvent * BulletCrashSound;
	UAkAudioEvent * EvolutionSound;
	UAkAudioEvent * ElectricSound;
	UAkAudioEvent * ItemSound;
	UAkAudioEvent * ItemGetSound;
};
