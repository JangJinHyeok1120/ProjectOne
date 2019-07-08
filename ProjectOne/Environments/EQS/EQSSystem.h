// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ProjectOne.h"
#include "GameFramework/Actor.h"
#include "EQSVariables.h"
#include "EQSSystem.generated.h"



UCLASS()
 class PROJECTONE_API AEQSSystem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEQSSystem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	class UNavigationSystemV1* navSys;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = EQS)
	FVector Center; //탐색 중심
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = EQS)
	float Radius; //반지름
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = EQS)
	float SpaceBetween; //Item 생성 간격

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = EQS)
	bool DrawDebug; //debug 그리기

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = EQS)
	bool Init; //Mapdata 생성

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = EQS)
	bool Save; //Mapdata 저장

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = EQS)
	bool Load; //MapData 로드

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = EQS)
	float Weight; //Astar 가중치





public:	

	TArray<FEQSItem> Items;
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//맵을 데이터화
	void InitSystem();

	void SaveData(TArray<FEQSItem>& items);
	void LoadData();

	//캐릭터 주변 맵데이터를 가져오는 함수
	int32 SearchMapData(float PosX, float PosY);

	//점수를 매김
	int32 SetScore(FVector Pos);

	//라인트레이스 점수를 매김
	int32 SetLineScore(FVector ItemPos, float LineLength);

	//캐릭터 주변 정보 가져오기 행 과 열이 같음 Length는 홀수만 가능
	TArray<FEQSItem> GetNearByCharacter(FVector ChPos,int Length);

	//Item 이웃들 가져오기
	TArray<FEQSItem> GetNeighborhood(int IndexNum, TArray<FEQSItem>& PathMap);

	//Path용 맵 데이터 가져오기
	TArray<FEQSItem> GetPathMap(FEQSItem StartItem,FEQSItem TargetItem);

	//Path 찾기
	TArray<FEQSItem> FindPath(FVector startPos, FVector targetPos);

	//주위에 숨은장소를 반환함
	FEQSItem GetHideSpot(FVector ChPos, AActor * Attacker, int SearchSize);

	bool IsWorking;

	//closedSet안에 특정 item이 있는지 검사
	bool Contains(TArray<FEQSItem>& list, FEQSItem item)
	{
		for (int i = 0; i < list.Num(); i++)
		{
			if (list[i].IndexNum == item.IndexNum)
				return true;
		}
		return false;
	}
};
