// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EQSVariables.generated.h"


USTRUCT(BlueprintType)
struct FEQSItem
{
	// SomWorks :D // GENERATED_USTRUCT_BODY() is Deprecated.
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EQSData")
	FVector Pos; //아이템 위치

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EQSData")
	bool bResult; //아이템이 갈수있는곳인지

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EQSData")
	int32 Score; //아이템 점수

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EQSData")
	int32 IndexNum; //아이템 인덱스번호

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EQSData")
	int32 ParentNum; //AStar용 부모 번호
};

USTRUCT(BlueprintType)
struct FMapNode
{
	GENERATED_BODY()

	//중심점
	UPROPERTY()
	int32 Center;
	//왼쪽위
	UPROPERTY()
	int32 LeftUp;
	//왼쪽아래
	UPROPERTY()
	int32 LeftDown;
	//오른쪽 위
	UPROPERTY()
	int32 RightUp;
	//오른쪽 아래
	UPROPERTY()
	int32 RightDown;
	//반지름
	UPROPERTY()
	int32 Radius;
};

enum ESearchDirection
{
	LEFT_UP,
	LEFT_DOWN,
	RIGHT_UP,
	RIGHT_DOWN
};

struct ItemNode
{
	FEQSItem item;
	float fCost;
	float gCost;
	float hCost;
};

class  PriorityQueue
{
public:

	 ItemNode* Queue;
	 int Count;

	PriorityQueue(int size)
	{
		Queue = new  ItemNode[size];
		Count = 0;
	}

	void Enqueue(ItemNode node)
	{
		Queue[Count++] = node;

		int Child = Count - 1;

		while (Child > 0)
		{
			int Parent = (Child - 1) / 2;

			if (CompareTo(Queue[Child],Queue[Parent]) > 0)
				break;

			Swap(Child, Parent);
			Child = Parent;
		}
	}

	void Swap(int n1, int n2)
	{
		ItemNode temp = Queue[n1];

		Queue[n1] = Queue[n2];
		Queue[n2] = temp;

	}

	ItemNode Dequeue()
	{
		ItemNode FirstNode = Queue[0];

		Queue[0] = Queue[--Count];

		int Parent = 0;

		while (true)
		{
			int Child = Parent * 2 + 1;

			if (Child > Count)
				break;

			int ChildNext = Child + 1;

			if (ChildNext <= Count && CompareTo(Queue[ChildNext],Queue[Child]) < 0)
				Child = ChildNext;

			if (CompareTo(Queue[Parent],Queue[Child]) <= 0)
				break;

			Swap(Parent, Child);

			Parent = Child;

		}

		return FirstNode;
	}

	bool Contains(ItemNode node)
	{
		for (int i = 0; i < Count; i++)
		{
			if (Queue[i].item.IndexNum == node.item.IndexNum)
				return true;
		}

		return false;
	}

	int CompareTo(ItemNode n1,ItemNode n2)
	{
		if (n1.fCost > n2.fCost)
			return 1;
		else if (n1.fCost == n2.fCost)
		{
			return 0;
		}
		else
			return -1;
	}

};