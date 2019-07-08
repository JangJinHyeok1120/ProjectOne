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
	FVector Center; //Ž�� �߽�
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = EQS)
	float Radius; //������
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = EQS)
	float SpaceBetween; //Item ���� ����

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = EQS)
	bool DrawDebug; //debug �׸���

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = EQS)
	bool Init; //Mapdata ����

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = EQS)
	bool Save; //Mapdata ����

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = EQS)
	bool Load; //MapData �ε�

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = EQS)
	float Weight; //Astar ����ġ





public:	

	TArray<FEQSItem> Items;
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//���� ������ȭ
	void InitSystem();

	void SaveData(TArray<FEQSItem>& items);
	void LoadData();

	//ĳ���� �ֺ� �ʵ����͸� �������� �Լ�
	int32 SearchMapData(float PosX, float PosY);

	//������ �ű�
	int32 SetScore(FVector Pos);

	//����Ʈ���̽� ������ �ű�
	int32 SetLineScore(FVector ItemPos, float LineLength);

	//ĳ���� �ֺ� ���� �������� �� �� ���� ���� Length�� Ȧ���� ����
	TArray<FEQSItem> GetNearByCharacter(FVector ChPos,int Length);

	//Item �̿��� ��������
	TArray<FEQSItem> GetNeighborhood(int IndexNum, TArray<FEQSItem>& PathMap);

	//Path�� �� ������ ��������
	TArray<FEQSItem> GetPathMap(FEQSItem StartItem,FEQSItem TargetItem);

	//Path ã��
	TArray<FEQSItem> FindPath(FVector startPos, FVector targetPos);

	//������ ������Ҹ� ��ȯ��
	FEQSItem GetHideSpot(FVector ChPos, AActor * Attacker, int SearchSize);

	bool IsWorking;

	//closedSet�ȿ� Ư�� item�� �ִ��� �˻�
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
