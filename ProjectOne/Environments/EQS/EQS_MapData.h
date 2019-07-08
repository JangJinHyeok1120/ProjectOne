// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ProjectOne.h"
#include "GameFramework/SaveGame.h"
#include "EQSVariables.h"
#include "EQS_MapData.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTONE_API UEQS_MapData : public USaveGame
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SaveData")
	FString SaveSlotName;		// Save game data file name

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "SaveData")
	int32 SaveIndex;		// Save game data file index

	UPROPERTY()
	TArray<FEQSItem> MapData;
};
