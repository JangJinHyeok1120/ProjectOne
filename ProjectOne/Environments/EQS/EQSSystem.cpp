// Fill out your copyright notice in the Description page of Project Settings.

#include "EQSSystem.h"
#include "EQS_MapData.h"
#include "DrawDebugHelpers.h"
#include "AI/AICharacter.h"
#include "Runtime/NavigationSystem/Public/NavigationSystem.h"
#include "Runtime/NavigationSystem/Public/NavigationPath.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AEQSSystem::AEQSSystem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Center = FVector(0, 0, 1260.0f);


}

// Called when the game starts or when spawned
void AEQSSystem::BeginPlay()
{
	Super::BeginPlay();

	navSys = UNavigationSystemV1::GetCurrent(GetWorld());

	Items.Reserve(1050625);

	if(Init)
	InitSystem();

	if(Load)
	LoadData();

	IsWorking = false;
	float startTime = 1.0f;

	for (FConstPawnIterator Iterator = GetWorld()->GetPawnIterator(); Iterator; ++Iterator)
	{
		auto AI = Cast<AAICharacter>(Iterator->Get());
		if (AI)
		{
			AI->EQSSystem = this;
			AI->StartTime = startTime;
			startTime += 0.5f;
		}
	}
}

// Called every frame
void AEQSSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEQSSystem::InitSystem()
{
	FHitResult OutHit;
	FCollisionQueryParams CollisionParams;

	float PosX = -18320.f;
	float PosY = 12830.0f;

	FVector StartPos(PosX, PosY, 27000.0f);
	FVector EndPos(PosX, PosY, -5000.0f);


	int ItemCount = Radius * 2 / SpaceBetween;
	int ItemCountHalf = ItemCount / 2;

	Items.Reserve(ItemCount * ItemCount);
		
		for (int32 y = 0; y <= ItemCount; y++)
		{
			for (int32 x = 0; x <= ItemCount; x++)
			{
				FEQSItem ItemTemp = { Center - FVector(SpaceBetween * (-x + ItemCountHalf), SpaceBetween * (y - ItemCountHalf), 0),false,0,Items.Num(),0 };
				Items.Add(ItemTemp);
			}
		}

	for (int x = 0; x < Items.Num(); x++)
	{
		StartPos = FVector(Items[x].Pos.X, Items[x].Pos.Y, 27000.f);
		EndPos = FVector(Items[x].Pos.X, Items[x].Pos.Y, -5000.0f);

		
		if (GetWorld()->LineTraceSingleByChannel(OutHit, StartPos, EndPos, ECC_GameTraceChannel3, CollisionParams))
		{
			if (OutHit.bBlockingHit)
			{
				UNavigationPath* NavPath = navSys->FindPathToLocationSynchronously(GetWorld(), Center, FVector(Items[x].Pos.X, Items[x].Pos.Y, OutHit.ImpactPoint.Z), NULL);

				if (NavPath->PathPoints.Num() > 0)
				{
					float TargetNum = NavPath->PathPoints.Num() - 1;
					if (FMath::Abs(Items[x].Pos.X - NavPath->PathPoints[TargetNum].X)<1.0f && FMath::Abs(Items[x].Pos.Y - NavPath->PathPoints[TargetNum].Y) <1.0f) {
						Items[x].Pos.Z = OutHit.ImpactPoint.Z;
						Items[x].bResult = true;
						Items[x].Score = SetLineScore(Items[x].Pos,300.0f);
					}
				}

			}


		}
	}


	if (DrawDebug) {
		for (int x = 0; x < Items.Num(); x++)
		{
			if (Items[x].bResult) 
			{
				if(Items[x].Score > 100)
					DrawDebugSphere(GetWorld(), Items[x].Pos, 10.0f, 4, FColor::Blue, false, 100.0f);
				else if (Items[x].Score > 0)
					DrawDebugSphere(GetWorld(), Items[x].Pos, 10.0f, 4, FColor::Cyan, false, 100.0f);
				else
					DrawDebugSphere(GetWorld(), Items[x].Pos, 10.0f, 4, FColor::Green, false, 100.0f);
			}
			else
				DrawDebugSphere(GetWorld(), Items[x].Pos, 10.0f, 4, FColor::Red, false, 100.0f);
		}
	}
	
	if(Save)
	SaveData(Items);
}

void AEQSSystem::SaveData(TArray<FEQSItem>& Data)
{
	UEQS_MapData * SaveMapInstance = Cast<UEQS_MapData>(UGameplayStatics::CreateSaveGameObject(UEQS_MapData::StaticClass()));

	if (SaveMapInstance)
	{
		SaveMapInstance->SaveSlotName = "EQS_MapData";
		SaveMapInstance->SaveIndex = 0;

		SaveMapInstance->MapData = Data;
	}

	UGameplayStatics::SaveGameToSlot(SaveMapInstance, SaveMapInstance->SaveSlotName, SaveMapInstance->SaveIndex);
}

void AEQSSystem::LoadData()
{
	UEQS_MapData * LoadMapInstance = Cast<UEQS_MapData>(UGameplayStatics::CreateSaveGameObject(UEQS_MapData::StaticClass()));

	if (LoadMapInstance)
	{
		LoadMapInstance->SaveSlotName = "EQS_MapData";
		LoadMapInstance->SaveIndex = 0;
	}

	LoadMapInstance = Cast<UEQS_MapData>(UGameplayStatics::LoadGameFromSlot(LoadMapInstance->SaveSlotName, LoadMapInstance->SaveIndex));

	Items = LoadMapInstance->MapData;


}

int32 AEQSSystem::SearchMapData(float PosX, float PosY)
{
	FMapNode Parents = {0,0,0,0,0,0};
	Parents.LeftUp = 0;
	Parents.RightDown = Items.Num() - 1;
	Parents.RightUp = (Radius * 2 / SpaceBetween);
	Parents.LeftDown = Parents.RightDown - Parents.RightUp;
	Parents.Center = (Parents.RightDown - Parents.LeftUp) / 2;
	Parents.Radius = (Parents.RightUp - Parents.LeftUp) / 2;

	ESearchDirection direction;

	bool bResult = false;

	while ((Parents.RightUp - Parents.LeftUp) / 2 > 0)
	{
		if (Items[Parents.Center].Pos.X == PosX && Items[Parents.Center].Pos.Y == PosY) 
		{
			bResult = true;
			break;
		}
		else if (Items[Parents.Center].Pos.X > PosX && Items[Parents.Center].Pos.Y < PosY)
		{
			direction = ESearchDirection::LEFT_UP;
		}
		else if(Items[Parents.Center].Pos.X > PosX && Items[Parents.Center].Pos.Y > PosY)
		{
			direction = ESearchDirection::LEFT_DOWN;
		}
		else if (Items[Parents.Center].Pos.X < PosX && Items[Parents.Center].Pos.Y < PosY)
		{
			direction = ESearchDirection::RIGHT_UP;
		}
		else if (Items[Parents.Center].Pos.X < PosX && Items[Parents.Center].Pos.Y > PosY)
		{
			direction = ESearchDirection::RIGHT_DOWN;
		}
		else if (Items[Parents.Center].Pos.X == PosX && Items[Parents.Center].Pos.Y > PosY)
		{
			direction = ESearchDirection::RIGHT_DOWN;
		}
		else if (Items[Parents.Center].Pos.X == PosX && Items[Parents.Center].Pos.Y < PosY)
		{
			direction = ESearchDirection::RIGHT_UP;
		}
		else if (Items[Parents.Center].Pos.X > PosX && Items[Parents.Center].Pos.Y == PosY)
		{
			direction = ESearchDirection::LEFT_UP;
		}
		else if (Items[Parents.Center].Pos.X < PosX && Items[Parents.Center].Pos.Y == PosY)
		{
			direction = ESearchDirection::RIGHT_UP;
		}

		switch (direction)
		{
		case ESearchDirection::LEFT_UP:
			Parents.LeftDown = Parents.Center - Parents.Radius;
			Parents.RightUp = Parents.RightUp - Parents.Radius;
			Parents.RightDown = Parents.Center;
			Parents.Center = (Parents.LeftUp + Parents.RightDown) / 2;
			Parents.Radius = (Parents.RightUp - Parents.LeftUp) / 2;
			break;
		case ESearchDirection::LEFT_DOWN:
			Parents.LeftUp = Parents.Center - Parents.Radius;
			Parents.RightUp = Parents.Center;
			Parents.RightDown = Parents.RightDown - Parents.Radius;
			Parents.Center = (Parents.LeftUp + Parents.RightDown) / 2;
			Parents.Radius = (Parents.RightUp - Parents.LeftUp) / 2;
			break;
		case ESearchDirection::RIGHT_UP:
			Parents.LeftUp = Parents.RightUp - Parents.Radius;
			Parents.LeftDown = Parents.Center;
			Parents.RightDown = Parents.Center + Parents.Radius;
			Parents.Center = (Parents.LeftUp + Parents.RightDown) / 2;
			Parents.Radius = (Parents.RightUp - Parents.LeftUp) / 2;
			break;
		case ESearchDirection::RIGHT_DOWN:
			Parents.LeftUp = Parents.Center;
			Parents.LeftDown = Parents.RightDown - Parents.Radius;
			Parents.RightUp = Parents.Center + Parents.Radius;
			Parents.Center = (Parents.LeftUp + Parents.RightDown) / 2;
			Parents.Radius = (Parents.RightUp - Parents.LeftUp) / 2;
			break;
		default:
			break;
		}
	}

	if (bResult) 
	{
		return Parents.Center;
	}
	else
	{
		if (Items[Parents.LeftUp].Pos.X == PosX && Items[Parents.LeftUp].Pos.Y == PosY && Items[Parents.LeftUp].bResult)
		{
			return Parents.LeftUp;
		}
		else if (Items[Parents.LeftDown].Pos.X == PosX && Items[Parents.LeftDown].Pos.Y == PosY && Items[Parents.LeftUp].bResult)
		{
			return Parents.LeftDown;
		}
		else if (Items[Parents.RightUp].Pos.X == PosX && Items[Parents.RightUp].Pos.Y == PosY && Items[Parents.LeftUp].bResult)
		{
			return Parents.RightUp;
		}
		else if (Items[Parents.RightDown].Pos.X == PosX && Items[Parents.RightDown].Pos.Y == PosY && Items[Parents.LeftUp].bResult)
		{
			return Parents.RightDown;
		}
		else
		{
			return Parents.RightDown;
		}
			
	}

}

int32 AEQSSystem::SetScore(FVector ItemPos)
{
	FHitResult OutHit;
	FCollisionQueryParams CollisionParams;
	
	int32 Score = 0;

	FVector LineStartPos = ItemPos + FVector(0, 0, 400);
	FVector LineEndPos = LineStartPos + FVector(200, 0, 0);
	DrawDebugLine(GetWorld(), LineStartPos, LineEndPos, FColor::Green, false, 20.0f, 0, 1);

	if (GetWorld()->LineTraceSingleByChannel(OutHit, LineStartPos, LineEndPos, ECC_GameTraceChannel3, CollisionParams))
	{
		if (OutHit.bBlockingHit)
		{
			if (OutHit.Actor->ActorHasTag(TEXT("Cover")))
			{
				Score += 200;
				Score -= OutHit.Distance;
			}
		}
	}

	 LineEndPos = LineStartPos + FVector(-200, 0, 0);

	if (GetWorld()->LineTraceSingleByChannel(OutHit, LineStartPos, LineEndPos, ECC_GameTraceChannel3, CollisionParams))
	{
		if (OutHit.bBlockingHit)
		{
			if (OutHit.Actor->ActorHasTag(TEXT("Cover")))
			{
				Score += 200;
				Score -= OutHit.Distance;
			}
		}
	}

	 LineEndPos = LineStartPos + FVector(0, 200, 0);

	if (GetWorld()->LineTraceSingleByChannel(OutHit, LineStartPos, LineEndPos, ECC_GameTraceChannel3, CollisionParams))
	{
		if (OutHit.bBlockingHit)
		{
			if (OutHit.Actor->ActorHasTag(TEXT("Cover")))
			{
				Score += 200;
				Score -= OutHit.Distance;
			}
		}
	}

	 LineEndPos = LineStartPos + FVector(0, -200, 0);

	if (GetWorld()->LineTraceSingleByChannel(OutHit, LineStartPos, LineEndPos, ECC_GameTraceChannel3, CollisionParams))
	{
		if (OutHit.bBlockingHit)
		{
			if (OutHit.Actor->ActorHasTag(TEXT("Cover")))
			{
				Score += 200;
				Score -= OutHit.Distance;
			}
		}
	}

	return Score;
}

int32 AEQSSystem::SetLineScore(FVector ItemPos, float LineLength)
{
	FHitResult OutHit;
	FCollisionQueryParams CollisionParams;

	int32 Score = 0;

	FVector LineStartPos = ItemPos + FVector(0, 0, 300);
	FVector LineEndPos = LineStartPos + FVector(LineLength, 0, 0);

	if (GetWorld()->LineTraceSingleByChannel(OutHit, LineStartPos, LineEndPos, ECC_GameTraceChannel3, CollisionParams))
	{
		if (OutHit.bBlockingHit)
		{
			if (OutHit.Actor->ActorHasTag(TEXT("Cover")))
			{
				Score += LineLength;
				Score -= OutHit.Distance;
			}
		}
	}

	LineEndPos = LineStartPos + FVector(-LineLength, 0, 0);


	if (GetWorld()->LineTraceSingleByChannel(OutHit, LineStartPos, LineEndPos, ECC_GameTraceChannel3, CollisionParams))
	{
		if (OutHit.bBlockingHit)
		{
			if (OutHit.Actor->ActorHasTag(TEXT("Cover")))
			{
				Score += LineLength;
				Score -= OutHit.Distance;
			}
		}
	}

	LineEndPos = LineStartPos + FVector(0, LineLength, 0);

	if (GetWorld()->LineTraceSingleByChannel(OutHit, LineStartPos, LineEndPos, ECC_GameTraceChannel3, CollisionParams))
	{
		if (OutHit.bBlockingHit)
		{
			if (OutHit.Actor->ActorHasTag(TEXT("Cover")))
			{
				Score += LineLength;
				Score -= OutHit.Distance;
			}
		}
	}

	LineEndPos = LineStartPos + FVector(0, -LineLength, 0);

	if (GetWorld()->LineTraceSingleByChannel(OutHit, LineStartPos, LineEndPos, ECC_GameTraceChannel3, CollisionParams))
	{
		if (OutHit.bBlockingHit)
		{
			if (OutHit.Actor->ActorHasTag(TEXT("Cover")))
			{
				Score += LineLength;
				Score -= OutHit.Distance;
			}
		}
	}




	LineEndPos = LineStartPos + FVector(LineLength, 0, 0);

	if (GetWorld()->LineTraceSingleByChannel(OutHit, LineStartPos, LineEndPos, ECC_GameTraceChannel4, CollisionParams))
	{
		if (OutHit.bBlockingHit)
		{
			Score += LineLength;
			Score -= OutHit.Distance;
		}
	}

	LineEndPos = LineStartPos + FVector(-LineLength, 0, 0);

	if (GetWorld()->LineTraceSingleByChannel(OutHit, LineStartPos, LineEndPos, ECC_GameTraceChannel4, CollisionParams))
	{
		if (OutHit.bBlockingHit)
		{
				Score += LineLength;
				Score -= OutHit.Distance;
		}
	}

	LineEndPos = LineStartPos + FVector(0, LineLength, 0);

	if (GetWorld()->LineTraceSingleByChannel(OutHit, LineStartPos, LineEndPos, ECC_GameTraceChannel4, CollisionParams))
	{
		if (OutHit.bBlockingHit)
		{
			Score += LineLength;
			Score -= OutHit.Distance;
		}
	}

	LineEndPos = LineStartPos + FVector(0, -LineLength, 0);

	if (GetWorld()->LineTraceSingleByChannel(OutHit, LineStartPos, LineEndPos, ECC_GameTraceChannel4, CollisionParams))
	{
		if (OutHit.bBlockingHit)
		{
			Score += LineLength;
			Score -= OutHit.Distance;
		}
	}

	return Score;
}

TArray<FEQSItem> AEQSSystem::GetNearByCharacter(FVector ChPos, int Length)
{
	TArray<FEQSItem> TempItems;

	TempItems.Reserve((Length + 1) * (Length + 1));

	int RowNum = Radius * 2 / SpaceBetween +1;

	int CenterNum = SearchMapData(ChPos.X, ChPos.Y);

	int Count = 0;

	for (int i = -((Length - 1) / 2); i <= ((Length - 1) / 2); i++)
	{
		for (int j = (CenterNum + (RowNum * i)) - ((Length - 1) / 2); j <= (CenterNum + RowNum * i) + ((Length - 1) / 2); j++)
		{
			TempItems.Add(Items[j]);
			Count++;
		}
	}

	return TempItems;
}

TArray<FEQSItem> AEQSSystem::GetNeighborhood(int IndexNum, TArray<FEQSItem>& PathMap)
{
	TArray<FEQSItem> Neighborhood;

	int MapLastIndex = PathMap.Num() - 1;
	int RowLength = 1025;

	for (int y = -1; y <= 1; y++)
	{
		for (int x = -1; x <= 1; x++)
		{
			if (!(x == 0 && y == 0))
				if ((PathMap[0].Pos.X <= Items[IndexNum + RowLength * y + x].Pos.X) 
					&& (Items[IndexNum + RowLength * y + x].Pos.X <= PathMap[MapLastIndex].Pos.X)
					&& (PathMap[MapLastIndex].Pos.Y <= Items[IndexNum + RowLength * y + x].Pos.Y)
					&& (Items[IndexNum + RowLength * y + x].Pos.Y <= PathMap[0].Pos.Y))
				{
					Neighborhood.Add(Items[IndexNum + RowLength * y + x]);
				}
				
		}
	}	

	return Neighborhood;
}

TArray<FEQSItem> AEQSSystem::GetPathMap(FEQSItem StartItem, FEQSItem TargetItem)
{
	TArray<FEQSItem> TempItems;

	int Length = FVector::Distance(StartItem.Pos, TargetItem.Pos) / SpaceBetween;

	Length+=5;

	if (Length % 2 == 0)
		Length -= 1;

	TempItems.Reserve((Length + 1) * (Length + 1));

	int RowNum = Radius * 2 / SpaceBetween + 1;

	int CenterNum = SearchMapData(StartItem.Pos.X +((TargetItem.Pos.X -StartItem.Pos.X )/2), StartItem.Pos.Y + ((TargetItem.Pos.Y - StartItem.Pos.Y) / 2));

	int Count = 0;

	for (int i = -((Length - 1) / 2); i <= ((Length - 1) / 2); i++)
	{
		for (int j = (CenterNum + (RowNum * i)) - ((Length - 1) / 2); j <= (CenterNum + RowNum * i) + ((Length - 1) / 2); j++)
		{
			TempItems.Add(Items[j]);
			Count++;
		}
	}

	return TempItems;
}

TArray<FEQSItem> AEQSSystem::FindPath(FVector startPos, FVector targetPos)
{
		ItemNode startNode = { Items[SearchMapData(startPos.X,startPos.Y)],0,0,0 };
		ItemNode targetNode = { Items[SearchMapData(targetPos.X,targetPos.Y)],0,0,0 };
		TArray<FEQSItem> PathList;

		bool Path = false;
		bool Path2 = false;

		if (!startNode.item.bResult || !targetNode.item.bResult)
		{
			return PathList;
		}


		
		TArray<FEQSItem> PathMap = GetPathMap(Items[startNode.item.IndexNum], Items[targetNode.item.IndexNum]);


		for (int i = 0; i < PathMap.Num() - 1; i++)
		{
			if (PathMap[i].IndexNum == targetNode.item.IndexNum)
				Path = true;


		}

		for (int i = 0; i < PathMap.Num() - 1; i++)
		{
			if (PathMap[i].IndexNum == startNode.item.IndexNum)
				Path2 = true;


		}

		if (!Path) {

			return PathList;

		}

		if (!Path2)
		{
			return PathList;
		}


		PriorityQueue openSet(PathMap.Num());
		TArray<FEQSItem> closedSet;
		openSet.Enqueue(startNode);


		PathList.Reserve(PathMap.Num());

		int count =0;

		while (openSet.Count > 0) {

			if (count >= 1000) {
				ABLOG(Warning, TEXT("CountFail"));
				return PathList;

			}

			count++;
			// OpenSet에서 가장 낮은 fCost를 가지는 노드를 가져온다. 
			// 만일 fCost가 동일할 경우 gCost가 적은 쪽을 택함. 
			ItemNode currentNode = openSet.Dequeue();
			// 찾은 노드가 최종 노드면 루프 종료
			if (currentNode.item.IndexNum == targetNode.item.IndexNum)
			{

				int endIndex = startNode.item.IndexNum;
				int curIndex = targetNode.item.IndexNum;

				int count = 0;
				while (curIndex != endIndex)
				{
					
					PathList.Add(Items[curIndex]);
					count++;
					curIndex = Items[curIndex].ParentNum;

				}

				return PathList;
			}


			closedSet.Add(currentNode.item);

			TArray<FEQSItem> neightbourList = GetNeighborhood(currentNode.item.IndexNum, PathMap);
			// 현재 노드의 이웃들을 모두 가져와 비교
			for (int i = 0; i < neightbourList.Num(); i++)
			{
				ItemNode n = { neightbourList[i],0,0,0 };

				if (Contains(closedSet,neightbourList[i]) || !n.item.bResult)
					continue;


				int g = FVector::Distance(currentNode.item.Pos, n.item.Pos) - n.item.Score * Weight;
				int h = FVector::Distance(n.item.Pos, targetNode.item.Pos);
				int f = g + h;

				if (!openSet.Contains(n))
				{
					n.gCost = g;
					n.hCost = h;
					n.fCost = f;
					Items[n.item.IndexNum].ParentNum= currentNode.item.IndexNum;
					openSet.Enqueue(n);
				}
				else
				{
					// 오픈셋에 현재 노드가 이미 있으면 수치를 비교한 후 경로를 교체한다. 동일하면 g수치가 큰 쪽으로 교체한다.
					if (n.fCost > f || (n.fCost == f && n.gCost > g))
					{
						n.gCost = g;
						n.hCost = h;
						n.fCost = f;
						Items[n.item.IndexNum].ParentNum = currentNode.item.IndexNum;
					}


				}
			}
		}

		ABLOG(Warning, TEXT("Fail5!!!! %d"),  PathMap.Num());
		return PathList;
		
}

FEQSItem AEQSSystem::GetHideSpot(FVector ChPos, AActor * Attacker, int SearchSize)
{
	auto Character = Cast<AProjectOneCharacter>(Attacker);

	TArray<FEQSItem> AroundItems = GetNearByCharacter(ChPos, SearchSize);

	FEQSItem HideSpot;
	FHitResult OutHit;
	FCollisionQueryParams CollisionParams;

	bool CanHide = false;


	for (int i = 0; i < AroundItems.Num() - 1; i++)
	{
		if (AroundItems[i].bResult) 
		{
			if (GetWorld()->LineTraceSingleByChannel(OutHit, Character->FollowCamera->GetComponentLocation(), FVector(AroundItems[i].Pos.X, AroundItems[i].Pos.Y, AroundItems[i].Pos.Z+50.0f), ECC_GameTraceChannel6, CollisionParams))
			{
				if (OutHit.bBlockingHit )
				{
					if (OutHit.Actor->ActorHasTag(TEXT("Cover"))){
						CanHide = true;

						if (FVector::Distance(OutHit.Actor->GetActorLocation(), HideSpot.Pos) > FVector::Distance(OutHit.Actor->GetActorLocation(), AroundItems[i].Pos))
							HideSpot = AroundItems[i];


					}
				}
				
			}
		}
		

	}

	HideSpot.bResult = CanHide;

		return HideSpot;
}




