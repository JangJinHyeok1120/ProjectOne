// Fill out your copyright notice in the Description page of Project Settings.

#include "AICharacter.h"
#include "Weapons/Bullet.h"
#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Runtime/NavigationSystem/Public/NavigationSystem.h"
#include "Runtime/NavigationSystem/Public/NavigationPath.h"
#include "Runtime/AIModule/Classes/Blueprint/AIBlueprintHelperLibrary.h"
#include "Components/WidgetComponent.h"
#include "Components/Widget.h"
#include "Player/POComponents/PlayerStatComponent.h"
#include "Weapons/SuperGun.h"
#include "Environments/EQS/EQSSystem.h"
#include "Environments/EQS/EQSVariables.h"
#include "Environments/GameModes/ProjectOneGameState.h"
#include "Item/SuperItem.h"
#include "Player/POComponents/PlayerStatComponent.h"
#include "ProjectOne/Environments/GameModes/ProjectOneGameInstance.h"
#include "Player/POComponents/InventoryComponent.h"
#include "ProjectOneAIController.h"

void AAICharacter::Tick(float delta)
{
	if(GetWorld()->GetTimeSeconds() > StartTime)
	Update();


	if (isShooting)
	{
		if (TargetPlayer != NULL) {
			LookAt(Prediction());
		}
		Shooting(delta);
	}

	if (Hiding)
	{
		if (Distance(HidePos,GetActorLocation()) <100)
		{
			HidePos = FVector::ZeroVector;
			CurState = AIState::Guard;
			LookAt(LastAttaker->GetActorLocation());
			LookAroundStart();
		}
	}

	if (IsLookAround)
	{
		LookAround();
	}
}

void AAICharacter::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimer(InitTime, this, &AAICharacter::InitTargetPlayer, 5.0f, false);
	GetWorld()->GetTimerManager().SetTimer(MagneticFieldTimer, this, &AAICharacter::CheckMagneticField, 1.0f, true);
	GetWorld()->GetTimerManager().SetTimer(ItemTimer, this, &AAICharacter::ItemCheck, 1.0f, true);
	GetWorld()->GetTimerManager().SetTimer(DetectedTimer, this, &AAICharacter::Detected, 0.1f, true);

	GetCharacterMovement()->SetWalkableFloorAngle(70.0f);
	CurState = AIState::Neutral;
	pathCount = -1;
	LookCount = 0;
	IsMoving = false;
	isFail = false;
	DelayTime = 3;
	isDelay = false;
	IsMoveToItem = false;
	CurrentMagneticFieldRadius = -1;
	IsFinal = false;

	MagneticField = Cast<AProjectOneGameState>(GetWorld()->GetGameState());

}

void AAICharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AAICharacter::Forward()
{
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	APAnim->Forward = 1.0f * 100.0f;
	// find out which way is forward
	const FRotator Rotation = AICon->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	// get forward vector
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	AddMovementInput(Direction, 1.0f);
}

void AAICharacter::Back()
{
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	APAnim->Forward = -1.0f * 100.0f;
	// find out which way is forward
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	// get forward vector
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(-Direction, 1.0f);
}

void AAICharacter::Left()
{
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	//ABLOG_S(Warning);
	APAnim->Right = -1.0f * 100.0f;
	APAnim->Right = -1.0f * 100.0f;

	// find out which way is right
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	// get right vector 
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	//add movement in that direction
	AddMovementInput(-Direction, 1.0f);

	FHitResult OutHit;
	FCollisionQueryParams CollisionParams;

	if (GetWorld()->LineTraceSingleByChannel(OutHit, GetActorLocation(), GetActorLocation() - GetActorRightVector() * 100, ECC_Visibility, CollisionParams))
	{
		LeftCheck = true;
		RigthCheck = false;
	}
}

void AAICharacter::RIght()
{
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	//ABLOG_S(Warning);
	APAnim->Right = 1.0f * 100.0f;

	// find out which way is right
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	// get right vector 
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	

	//add movement in that direction
	AddMovementInput(Direction, 1.0f);

	FHitResult OutHit;
	FCollisionQueryParams CollisionParams;

	if (GetWorld()->LineTraceSingleByChannel(OutHit, GetActorLocation(), GetActorLocation()+GetActorRightVector()*100, ECC_Visibility, CollisionParams))
	{
		RigthCheck = true;
		LeftCheck = false;
	}

}

void AAICharacter::LookAt(FVector LookPos)
{
	const FVector Camera = FollowCamera->GetComponentLocation();
	const FRotator rot = FRotationMatrix::MakeFromX(LookPos - Camera).Rotator();


	GetController()->SetControlRotation(rot);
}

void AAICharacter::LookAround()
{
	if (IsLookAround) {

		GetCharacterMovement()->bUseControllerDesiredRotation = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;

		if (!LeftEnd)
		{
			GetController()->SetControlRotation(FRotator(0, FMath::Lerp(GetController()->GetControlRotation().Yaw, LeftAngle, 0.01f), 0));

			

			if (FMath::Abs(GetController()->GetControlRotation().Yaw - LeftAngle) < 20.0f) {
				LeftEnd = true;
				LookCount++;
			}
		}
		else
		{
			GetController()->SetControlRotation(FRotator(0, FMath::Lerp(GetController()->GetControlRotation().Yaw, RightAngle, 0.01f), 0));
			if (FMath::Abs(GetController()->GetControlRotation().Yaw - RightAngle) < 20.0f) {
				LeftEnd = false;
				LookCount++;
			}
		}

		if (LookCount > 1)
		{
			LookCount = 0;
			IsLookAround = false;
			MoveEnd = false;
			pathCount--;
		}

	}

}

void AAICharacter::LookAroundStart()
{
	switch (CurState)
	{
	case Neutral:
		if (Hiding) {
			LookAt(LastAttaker->GetActorLocation());
			LeftAngle = GetController()->GetControlRotation().Yaw - 60.0f;
			RightAngle = GetController()->GetControlRotation().Yaw + 60.0f;
			AllStop();
			Hiding = false;
			IsLookAround = true;
			LeftEnd = false;
			LookCount = -3;
		}
		else
		{
			IsLookAround = true;
			LeftAngle = GetController()->GetControlRotation().Yaw - 60.0f;
			RightAngle = GetController()->GetControlRotation().Yaw + 60.0f;
			LeftEnd = false;
		}
		break;
	case Offensive:
		break;
	case Diffensive:
		break;
	case Guard:
		if (Hiding) {
			LookAt(LastAttaker->GetActorLocation());
			LeftAngle = GetController()->GetControlRotation().Yaw - 180.0f;
			RightAngle = GetController()->GetControlRotation().Yaw + 180.0f;
			AllStop();
			Hiding = false;
			IsLookAround = true;
			LeftEnd = false;
			LookCount = -3;
		}
		else
		{
			IsLookAround = true;
			LeftAngle = GetController()->GetControlRotation().Yaw - 60.0f;
			RightAngle = GetController()->GetControlRotation().Yaw + 60.0f;
			LeftEnd = false;
		}

		break;
	default:
		break;
	}

	
	
}

void AAICharacter::Shooting(float tick)
{
	if (Gun->GetCurBullet() <= 0) {
		ReloadAnimPlay();
		isShooting = false;
	}
	Gun->Fire(GetMesh()->GetSocketLocation(TEXT("Muzzle")), true);
}

FVector AAICharacter::Prediction()
{
	if (TargetPlayer == NULL)
		return FVector::ZeroVector;

	

	if (TargetPlayer == NULL)
		return FVector::ZeroVector;

	if (!TargetPlayer->IsAlive)
		return FVector::ZeroVector;

	float distance = 0;

	if (TargetPlayer->IsAlive&& TargetPlayer != NULL) {
		distance = FVector::Distance(GetMesh()->GetSocketLocation(TEXT("Muzzle")), TargetPlayer->GetActorLocation());
	}
	else
	{
		return FVector::ZeroVector;
	}

	if (!TargetPlayer->IsAlive)
		return FVector::ZeroVector;

	float test;

	if (TargetPlayer->IsAlive && TargetPlayer !=NULL) {
		test = (TargetPlayer->GetVelocity() - (FollowCamera->GetForwardVector() * CharacterStat->GetWeaphoneStatData()->BulletSpeed)).Size();
	}
	else
	{
		return FVector::ZeroVector;
	}


	float time = distance / test;

	if (!TargetPlayer->IsAlive)
		return FVector::ZeroVector;

	FVector LookVec;

	if (TargetPlayer->IsAlive) {
		 LookVec = TargetPlayer->GetActorLocation() + time * TargetPlayer->GetVelocity();
	}
	else
	{
		return FVector::ZeroVector;
	}

	return LookVec;
}

void AAICharacter::Update()
{
	if (IsAlive) {
		switch (CurState)
		{
		case AIState::Neutral:
			GetCharacterMovement()->MaxWalkSpeed = 600.0f;

			if (isShooting)
				Shot();

			if (MagneticField->TargetCircleRadius != 0) 
			{

				if (TargetPos != FVector::ZeroVector)
				{
					if (CheckLocation(TargetPos))
					{
						AllStop();
						TargetPos = GetInsidePos();
						IsMoving = true;
						IsMoveToItem = false;
						isFail = false;
					}
				}



				if (TargetPos == FVector::ZeroVector && MagneticField != nullptr)
				{

					if (MagneticField->TargetCircleRadius != 0)
					{
						TargetPos = GetInsidePos();
						CurrentMagneticFieldRadius = MagneticField->TargetCircleRadius;
						IsMoving = true;
					}
				}


				if (IsMoving && isInside && !IsMoveToItem) {
					AstarMove(TargetPos);
				}
				else if (!isInside)
				{
					AICon->MoveToLocation(TargetPos);
				}
				else if (!IsMoving && !IsMoveToItem)
				{
					TargetPos = GetInsidePos();
					IsMoving = true;
					isFail = false;
				}

				if (isFail && !IsMoveToItem)
				{
					TargetPos = GetInsidePos();
					IsMoving = true;
					isFail = false;
				}

			}
			else
			{
				if (IsFinal && TargetPos != FVector::ZeroVector)
				{
					AICon->MoveToLocation(TargetPos);
				}

				if (TargetPos == FVector::ZeroVector)
				{
					IsFinal = false;
				}

				if (CheckLocation(TargetPos))
				{
					IsFinal = false;
					
				}
			}

			break;
		case AIState::Offensive:

			AICon->StopMovement();
			GetCharacterMovement()->MaxWalkSpeed = 450.0f;
			if (!isShooting)
				Shot();

			if (TargetPlayer == NULL)
			{
				Shot();
				PastCovers.Empty();
				MoveEnd = false;
				AllStop();
				IsMoveToItem = false;

				IsMoving = true;
				GetCharacterMovement()->MaxWalkSpeed = 600.0f;
				isFail = false;
				CurState = AIState::Neutral;
				return;
			}


			if (TargetPlayer != NULL) {


				if (IsPattern)
				{
					Pattern();
				}

				if (!IsPattern) {
					if (!isDelay)
					{
						SetPattern();
					}

					if (TargetPlayer->IsAlive) {

						if (Distance(GetActorLocation(), TargetPlayer->GetActorLocation()) > 1500.0f)
							Forward();
						else {
							if (RigthCheck)
								Left();
							else if (!RigthCheck)
								RIght();
							else if (RigthCheck && LeftCheck)
								Forward();
						}
						if (CharacterStat->CurHP<40.0f && TargetPlayer->CharacterStat->CurHP > CharacterStat->CurHP) {
							IsMoveToItem = false;
							CurState = AIState::Diffensive;
							RigthCheck = false;
						}
					}
					

				}
			}
			break;
		case AIState::Diffensive:

			if (TargetPlayer == NULL)
			{
				Shot();
				PastCovers.Empty();
				MoveEnd = false;
				AllStop();
				IsMoveToItem = false;
				IsMoving = true;
				GetCharacterMovement()->MaxWalkSpeed = 600.0f;
				isFail = false;
				CurState = AIState::Neutral;
				return;
			}

			if (TargetPlayer != NULL) {

				if (!isShooting)
					Shot();

				if (TargetPlayer->IsAlive) {
					LookAt(TargetPlayer->GetActorLocation());
					Back();
				}
			}

			break;
		case AIState::Guard:
			if (TargetPlayer != NULL) {

				if (IsMoveToLastAttaker)
				{
					AllStop();
					IsMoveToItem = false;
					CurState = Offensive;
				}

				if (isGuard)
				{
					isGuard = false;
					GetWorldTimerManager().ClearTimer(GuardTimerHandle);
				}

				if (!Hiding)
				{
					if (!isShooting)
						Shot();
					GetCharacterMovement()->bUseControllerDesiredRotation = true;
					GetCharacterMovement()->bOrientRotationToMovement = false;
				
				}
			}
			else {

				if (IsMoveToLastAttaker)
				{
					if (CheckLocation(LastAttakerPos))
					{
						AllStop();
						IsMoveToItem = false;
						IsMoving = true;
						GetCharacterMovement()->MaxWalkSpeed = 600.0f;
						isFail = false;
						CurState = AIState::Neutral;
					}
				}

				if (!isGuard)
				{
					isGuard = true;
					GuardTime = 6;
					GetWorld()->GetTimerManager().SetTimer(GuardTimerHandle, this, &AAICharacter::GuardTimer, 1.0f, true);
				}

				if (isAim)
					Aim();
			}
			break;
		default:
			break;
		}
	}
	else
		isShooting = false;
}

void AAICharacter::MoveToLocation(FVector Location)
{
	if (!CheckLocation(Location)) {


		TargetLocationPos = Location;

		if (DetectedCovers.Num() > 1 && TargetCover) {
			NextCover(Location);
			MoveCover();
		}
		else
		{
			AICon->MoveToLocation(TargetLocationPos);
		}
	}
	else if (Hiding)
	{
		Hiding = false;
		MoveEnd = true;
	}
	else
	{
		MoveEnd = true;
		LookAroundStart();
	}
}

void AAICharacter::Hide(AActor * Attacker)
{
	FEQSItem hideSpot;

	switch (CurState)
	{
	case Neutral:
		hideSpot = EQSSystem->GetHideSpot(GetActorLocation(), Attacker, 17);

		if (hideSpot.bResult) {
				HidePos = hideSpot.Pos;
				AICon->MoveToLocation(hideSpot.Pos);
				LastAttaker = Attacker;
				Hiding = true;
				DelayTime = 3;
				LastAttakerPos = Attacker->GetActorLocation();
				GetWorld()->GetTimerManager().SetTimer(Delay, this, &AAICharacter::DelayTimer, 1.0f, true);
		}
		else
		{
			if (Distance(GetActorLocation(), Attacker->GetActorLocation()) > DetectedRange) {
				IsMoveToItem = false;
				AICon->MoveToLocation(Attacker->GetActorLocation());
				return;
			}
			else
			{
				AllStop();
				IsMoveToItem = false;
				IsMoving = true;
				LookAt(Attacker->GetActorLocation());
			}

			
		}
		break;
	case Offensive:
		break;
	case Diffensive:
		break;
	case Guard:
		hideSpot = EQSSystem->GetHideSpot(GetActorLocation(), Attacker, 21);

		if (hideSpot.bResult)
		{
				HidePos = hideSpot.Pos;
				AICon->MoveToLocation(hideSpot.Pos);
				LastAttaker = Attacker;
				Hiding = true;
				DelayTime = 3;
				isDelay = true;
				LastAttakerPos = Attacker->GetActorLocation();
				GetWorld()->GetTimerManager().SetTimer(Delay, this, &AAICharacter::DelayTimer, 1.0f, true);
			}

			
		else
		{
			if (Distance(GetActorLocation(), Attacker->GetActorLocation()) > DetectedRange) {
				AllStop();
				IsMoveToItem = false;
				IsMoving = true;
				GetCharacterMovement()->MaxWalkSpeed = 600.0f;
				isFail = false;
				AICon->MoveToLocation(Attacker->GetActorLocation());
				CurState = AIState::Neutral;
				return;
			}
			else
			{
				AllStop();
				LookAt(Attacker->GetActorLocation());
				IsMoveToItem = false;
				CurState = Offensive;
			}
			
		}
		break;

	default:
		break;
	}

	

}


void AAICharacter::Attacked(AActor * Attacker)
{
	switch (CurState)
	{
	case AIState::Neutral : 
		if (!isDelay) {
			Hide(Attacker);
		}
		else if (Distance(GetActorLocation(), Attacker->GetActorLocation()) < DetectedRange)
		{


			AllStop();
			LookAt(Attacker->GetActorLocation());
			CurState = Offensive;

		}

		break;

	case AIState::Guard:
		if (!isDelay) {

			if (!isAim)
				Aim();

			TargetPlayer = NULL;

			Hide(Attacker);

		}
		else if(Distance(GetActorLocation(),Attacker->GetActorLocation()) < DetectedRange)
		{


			AllStop();
			LookAt(Attacker->GetActorLocation());
			CurState = Offensive;

		}
		break;

	default:
		break;
	}
}

void AAICharacter::FindPlayer()
{
	switch (CurState)
	{
	case Neutral:
		if (TargetPlayer->IsAlive) {
			if (Distance(TargetPlayer->GetActorLocation(), GetActorLocation()) > 6000.0f)
			{
				return;
			}
			else {
				auto targetPlayer = Cast<AAICharacter>(TargetPlayer);

				if (targetPlayer)
				{
					if (targetPlayer->TargetPlayer == this) {

						if (IsLookAround)
							AllStop();
						isFail = false;
						IsFinal = false;
						IsMoveToItem = false;
						CurState = AIState::Offensive;

					}
					else
						if (!Hiding) {
							if (IsLookAround)
								AllStop();

							IsMoveToItem = false;
							CurState = AIState::Offensive;
							isFail = false;
							IsFinal = false;
						}
				}
				else
				{
					if (!Hiding) {
						if (IsLookAround)
							AllStop();

						IsMoveToItem = false;
						CurState = AIState::Offensive;
						isFail = false;
						IsFinal = false;
					}
				}
			}
		}
		break;
	case Offensive:
		break;
	case Diffensive:
		break;
	case Guard:
		if (TargetPlayer->IsAlive) {

			auto targetPlayer = Cast<AAICharacter>(TargetPlayer);

			if (targetPlayer)
			{

				if (Distance(TargetPlayer->GetActorLocation(), GetActorLocation()) < 3000.0f)
				{

					if (IsLookAround)
						AllStop();

					if (!isAim)
						Aim();
				}

			}
			else
			{
				if (Distance(TargetPlayer->GetActorLocation(), GetActorLocation()) < 3000.0f) {
	
					if (IsLookAround)
						AllStop();

					if (!isAim)
						Aim();
				}
			}
		}

		break;
	}
}

void AAICharacter::InitTargetPlayer()
{
	TargetPlayer = NULL;
	Detected();
	GetWorld()->GetTimerManager().SetTimer(InitTime, this, &AAICharacter::InitTargetPlayer, 5.0f, false);
}

void AAICharacter::AstarMove(FVector MovePoint)
{
	if (isFail)
		return;

	if (EQSSystem != nullptr)
	{
		if (pathCount <0 && IsMoving)
		{
			Path.Empty();
			Path = EQSSystem->FindPath(GetActorLocation(), MovePoint);

			if (Path.Num() != 0)
			{
				pathCount = Path.Num() - 1;
			}

			if (Path.Num() == 0)
			{
				TargetPos = GetInsidePos();
				AICon->MoveToLocation(TargetPos);
				isFail = true;
			}


		}
		else 
		{
			if (Path.Num() != 0) 
			{
				if (!CheckLocation(Path[pathCount].Pos)) 
				{
					AICon->MoveToLocation(Path[pathCount].Pos);
				}
				else
				{
					if (pathCount < 0)
					{
						AllStop();
						pathCount = -1;
						IsMoving = true;
						isFail = false;
						TargetPos = GetInsidePos();
						Path.Empty();
						return;
					}

					if (Path[pathCount].Score > 200 && !MoveEnd  && !isDelay && MagneticField->TargetCircleRadius !=0) {
						MoveEnd = true;
						LookAroundStart();
						GetWorld()->GetTimerManager().SetTimer(Delay, this, &AAICharacter::DelayTimer, 1.0f, true);
					}

					if (!MoveEnd)
					{
						pathCount--;
					}
					else
					{
						LookAround();
					}
				}
			}
		}
	}
}

FEQSItem AAICharacter::GetBestPos(FVector SearchCenter, int SearchSize)
{
	FEQSItem BestPos = { FVector::ZeroVector,false,-100,0,0 };
	if (EQSSystem != nullptr) {
		TArray<FEQSItem> AroundItems = EQSSystem->GetNearByCharacter(SearchCenter, SearchSize);

		for (int i = 0; i < AroundItems.Num() - 1; i++)
		{
			if (AroundItems[i].bResult)
			{
				if (PastPos.Contains(AroundItems[i].Pos))
					continue;

				if (BestPos.Score == -100)
				{
					BestPos = AroundItems[i];
				}


				if (BestPos.Score <= AroundItems[i].Score)
				{
					if (BestPos.Score == AroundItems[i].Score)
					{
						if (MagneticField->CurrentCirclePosition != FVector::ZeroVector)
						{
							if (Distance(MagneticField->CurrentCirclePosition, BestPos.Pos) > Distance(MagneticField->CurrentCirclePosition, AroundItems[i].Pos))
								BestPos = AroundItems[i];
						}
					}
					else
					{
						BestPos = AroundItems[i];
					}

				}
			}
		}
	}
	return BestPos;
}

void AAICharacter::MovingInside()
{
	if (EQSSystem != nullptr)
	{
		if (!IsMoving)
		{
				TargetPos = GetBestPos(GetActorLocation(), 11).Pos;
				IsMoving = true;
		}
		else
		{
			AstarMove(TargetPos);
		}

	}
}

FVector AAICharacter::GetInsidePos()
{
	if (MagneticField->TargetCircleRadius != 0)
	{
	
		FEQSItem TargetPos = { FVector::ZeroVector,false,-100,0,0 };

		while (!TargetPos.bResult) 
		{
			int angle = FMath::RandRange(0, 360);
			int distance = FMath::RandRange(1000, 2000);

			
			TargetPos = GetBestPos(MagneticField->TargetCirclePosition + FRotator(0.0f, angle, 0.0f).Vector().GetSafeNormal() * (MagneticField->TargetCircleRadius - distance),11);
		}
		return TargetPos.Pos;
	}
	return FVector::ZeroVector;
}

FVector AAICharacter::GetInsideNearestPos()
{
	if (MagneticField->TargetCircleRadius != 0)
	{

		FEQSItem TargetPos = { FVector::ZeroVector,false,-100,0,0 };
		int count = 0;

		while (!TargetPos.bResult)
		{
			if (count == 6)
			{
				return GetBestPos(MagneticField->TargetCirclePosition, 22).Pos;
			}

			int distance = FMath::RandRange(1000, 2000);


			TargetPos = GetBestPos(MagneticField->TargetCirclePosition +(GetActorLocation() - MagneticField->TargetCirclePosition).GetSafeNormal() * (MagneticField->TargetCircleRadius - distance), 11);
			count++;
		}
		return TargetPos.Pos;
	}
	return FVector::ZeroVector;
}

void AAICharacter::DelayTimer()
{
	isDelay = true;
	DelayTime--;


	if (DelayTime < 1)
	{
		DelayTime = 6;
		DelayCount++;
		isDelay = false;
		GetWorldTimerManager().ClearTimer(Delay);

	}
}

void AAICharacter::FindItem(AActor *DeitemPos)
{
	if (Inventory->CurItemCount < 3) {

		AllStop();
		AICon->MoveToActor(DeitemPos);
	}


}

void AAICharacter::AllStop()
{
	AICon->StopMovement();
	Path.Empty();
	pathCount = -1;

	IsLookAround = false;
	LookCount = 0;
	IsMoving = false;
	MoveEnd = false;
	IsMoveToLastAttaker = false;
	Hiding = false;
	IsPattern = false;
	isGuard = false;
	HidePos = FVector::ZeroVector;
	isShooting = false;
	
}

void AAICharacter::SetTargetPos()
{
	if (!IsMoving && MagneticField != nullptr)
	{
		TargetPos = GetInsidePos();
		IsMoving = true;
	}
}

void AAICharacter::CheckMagneticField()
{
	if (MagneticField != nullptr) 
	{
		if (MagneticField->TargetCircleRadius != 0)
		{
			if (Distance(GetActorLocation(), MagneticField->TargetCirclePosition) > MagneticField->CurrentCircleRadius - 4000.0f)
			{
				if (Distance(GetActorLocation(), MagneticField->TargetCirclePosition) > MagneticField->TargetCircleRadius)
				{
					if (MagneticField->TargetCircleRadius != 0)
					{
						if (isInside) {
							AllStop();

							if (isAim)
							{
								Aim();
							}
							TargetPos = GetInsideNearestPos();
							AICon->MoveToLocation(TargetPos);
							isInside = false;
						}
					}
					else
					{
						AllStop();
						IsMoving = true;
						TargetPos = GetBestPos(MagneticField->TargetCirclePosition, 44).Pos;
						isInside = false;
					}
				}
			}
			else
			{
				isInside = true;
			}
		}
		else
		{
			if (CurState == Neutral) {
				if (!IsFinal) {
					IsFinal = true;
					TargetPos = GetBestPos(MagneticField->TargetCirclePosition, 21).Pos;
				}
			}	
		}
	}
}

void AAICharacter::ItemCheck()
{
	if (!IsAlive)
		return;

	if (CharacterStat->CurHP < CharacterStat->GetMaxHP() *0.7f)
	{
		for (int i = 0; i < Inventory->Items.Num(); i++)
		{
			if (Inventory->eItems[i] == E_Item::E_HEAL_100 || 
				Inventory->eItems[i] == E_Item::E_HEAL_150 ||
				Inventory->eItems[i] == E_Item::E_HEAL_50)
			{
				if (CharacterStat->CurHP < 100.0f) {
					Inventory->Items[i]->Use();
					Inventory->Items[i] = nullptr;
					Inventory->eItems[i] = E_Item::E_NONE;
					Inventory->CurItemCount--;
				}
			}
		}
	}


	if (Inventory->CurItemCount > 0) {
		for (int i = 0; i < Inventory->Items.Num(); i++)
		{
			if (Inventory->eItems[i] == E_Item::E_EVOLUTION)
			{
					Inventory->Items[i]->Use();
					Inventory->Items[i] = nullptr;
					Inventory->eItems[i] = E_Item::E_NONE;
					Inventory->CurItemCount--;
			}
		}
	}
	

	if (!isInside)
	{
		for (int i = 0; i < Inventory->Items.Num(); i++)
		{
			if (Inventory->eItems[i] == E_Item::E_SPEED_BUFF)
			{
				Inventory->Items[i]->Use();
				Inventory->Items[i] = nullptr;
				Inventory->eItems[i] = E_Item::E_NONE;
				Inventory->CurItemCount--;
			}
		}
	}

	if (CurState == AIState::Offensive  || CurState == AIState::Diffensive)
	{
		for (int i = 0; i < Inventory->Items.Num(); i++)
		{
			if (Inventory->eItems[i] == E_Item::E_SPEED_BUFF || Inventory->eItems[i] == E_Item::E_AP_BUFF)
			{
				Inventory->Items[i]->Use();
				Inventory->Items[i] = nullptr;
				Inventory->eItems[i] = E_Item::E_NONE;
				Inventory->CurItemCount--;
			}
		}
	}


}

void AAICharacter::Pattern()
{
	switch (CurPattern)
	{
	case Pattern1:
		if (!isDelay &&DelayCount ==0) {
			DelayTime = 2;
			isDelay = true;
			GetWorld()->GetTimerManager().SetTimer(Delay, this, &AAICharacter::PatternTimer, 0.1f, true);
		}
		else if(!isDelay &&DelayCount ==1)
		{
			DelayTime = 3;
			DelayCount = 0;
			IsPattern = false;
			isDelay = true;
			GetWorld()->GetTimerManager().SetTimer(Delay, this, &AAICharacter::DelayTimer, 1.0f, true);

		}
		break;
	case Pattern2:
		if (RigthCheck) {
			RigthCheck = false;
		}
		else if (!RigthCheck)
			RigthCheck = true;

		DelayTime = 3;
		isDelay = true;
		GetWorld()->GetTimerManager().SetTimer(Delay, this, &AAICharacter::DelayTimer, 1.0f, true);
		IsPattern = false;
		break;

	case Pattern3:
		if (!isDelay &&DelayCount == 0) {
			RigthCheck = !RigthCheck;
			DelayTime = 1.3;
			isDelay = true;
			GetWorld()->GetTimerManager().SetTimer(Delay, this, &AAICharacter::PatternTimer, 0.1f, true);
		}
		else if (!isDelay &&DelayCount == 1)
		{
			RigthCheck = !RigthCheck;
			DelayTime = 1.5;
			isDelay = true;
			GetWorld()->GetTimerManager().SetTimer(Delay, this, &AAICharacter::PatternTimer, 0.1f, true);
		}
		else if (!isDelay &&DelayCount == 2)
		{
			RigthCheck = !RigthCheck;
			DelayTime = 1.2;
			isDelay = true;
			GetWorld()->GetTimerManager().SetTimer(Delay, this, &AAICharacter::PatternTimer, 0.1f, true);
		}
		else if (!isDelay &&DelayCount == 3)
		{
			RigthCheck = !RigthCheck;
			DelayTime = 1.4;
			isDelay = true;
			GetWorld()->GetTimerManager().SetTimer(Delay, this, &AAICharacter::PatternTimer, 0.1f, true);
		}
		else if (!isDelay &&DelayCount == 4)
		{
			RigthCheck = !RigthCheck;
			DelayTime = 3;
			isDelay = true;
			GetWorld()->GetTimerManager().SetTimer(Delay, this, &AAICharacter::DelayTimer, 1.0f, true);
			IsPattern = false;
			DelayCount = 0;
		}

		if (RigthCheck)
			Left();
		else if (!RigthCheck)
			RIght();
		break;

	case Pattern4:
		if (!isDelay &&DelayCount == 0) {
			RigthCheck = !RigthCheck;
			DelayTime = 1.3;
			isDelay = true;
			GetWorld()->GetTimerManager().SetTimer(Delay, this, &AAICharacter::PatternTimer, 0.1f, true);
		}
		else if (!isDelay &&DelayCount == 1)
		{
			RigthCheck = !RigthCheck;
			DelayTime = 1.5;
			isDelay = true;
			GetWorld()->GetTimerManager().SetTimer(Delay, this, &AAICharacter::PatternTimer, 0.1f, true);
		}
		else if (!isDelay &&DelayCount == 2)
		{
			RigthCheck = !RigthCheck;
			DelayTime = 1.2;
			isDelay = true;
			GetWorld()->GetTimerManager().SetTimer(Delay, this, &AAICharacter::PatternTimer, 0.1f, true);
		}
		else if (!isDelay &&DelayCount == 3)
		{
			RigthCheck = !RigthCheck;
			DelayTime = 1.4;
			isDelay = true;
			GetWorld()->GetTimerManager().SetTimer(Delay, this, &AAICharacter::PatternTimer, 0.1f, true);
		}
		else if (!isDelay &&DelayCount == 4)
		{
			RigthCheck = !RigthCheck;
			DelayTime = 3;
			isDelay = true;
			GetWorld()->GetTimerManager().SetTimer(Delay, this, &AAICharacter::DelayTimer, 1.0f, true);
			IsPattern = false;
			DelayCount = 0;
		}

		if (RigthCheck)
			Left();
		else if (!RigthCheck)
			RIght();

		Forward();
	default:
		break;
	}
}

void AAICharacter::SetPattern()
{
	int rand = FMath::RandRange(0, 9);

	if (rand >= 4)
		IsPattern = true;
	else
		IsPattern = false;

	if (IsPattern)
	{
		rand = FMath::RandRange(0, 9);

		if (rand <= 2)
		{
			CurPattern = Pattern1;
		}
		else if (rand <= 5)
		{
			CurPattern = Pattern2;
		}
		else
		{
			CurPattern = Pattern3;
		}


		if (Distance(GetActorLocation(), TargetPlayer->GetActorLocation()) > 1500.0f)
		{
			CurPattern = Pattern4;
		}
		DelayCount = 0;
	}
	else
	{
		DelayTime = 3;
		GetWorld()->GetTimerManager().SetTimer(Delay, this, &AAICharacter::DelayTimer, 1.0f, true);
	}


}

void AAICharacter::PatternTimer()
{
	isDelay = true;
	DelayTime-=0.1;


	if (DelayTime < 1)
	{
		DelayTime = 6;
		DelayCount++;
		isDelay = false;
		GetWorldTimerManager().ClearTimer(Delay);

	}
}

void AAICharacter::SetGuardPattern()
{
	int rand = FMath::RandRange(0, 1);

	if (rand == 0)
	{
		AllStop();
		IsMoving = true;
		IsMoveToItem = false;
		isFail = false;
		CurState = Neutral;
	}
	else
	{
		IsMoveToLastAttaker = true;
		AICon->MoveToLocation(LastAttakerPos);
	}
}

void AAICharacter::GuardTimer()
{
	isGuard = true;
	GuardTime--;


	if (GuardTime < 1)
	{
		GuardTime = 6;
		isGuard = false;
		SetGuardPattern();
		GetWorldTimerManager().ClearTimer(GuardTimerHandle);

	}
}



void AAICharacter::Detected()
{
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	// get forward vector
	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams CollisionQueryParam(NAME_None, false, this);
	bool bResult = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		GetActorLocation() + Direction * DetectedRange,
		FQuat::Identity,
		ECollisionChannel::ECC_Pawn,
		FCollisionShape::MakeSphere(DetectedRange),
		CollisionQueryParam
	);

	FHitResult OutHit;
	FHitResult LeftHit;
	FCollisionQueryParams CollisionParams;

	if (bResult)
	{
		int i = 0;
		for (auto OverlapResult : OverlapResults)
		{
			AProjectOneCharacter * Player = Cast<AProjectOneCharacter>(OverlapResult.GetActor());
			ASuperItem * DetectedItem = Cast<ASuperItem>(OverlapResult.GetActor());
			if (Player) {
				if (Player->IsAlive) {
					if (GetWorld()->LineTraceSingleByChannel(OutHit, FollowCamera->GetComponentLocation() + FollowCamera->GetForwardVector()*200.0f, Player->GetActorLocation(), ECC_Visibility, CollisionParams))
					{
						if (OutHit.bBlockingHit)
						{
							//ABLOG_S(Warning);
						}
					}
					else
					{
						if (!TargetPlayer)
							TargetPlayer = Player;

						if (TargetPlayer) {
							if (Distance(GetActorLocation(), Player->GetActorLocation()) < Distance(GetActorLocation(), TargetPlayer->GetActorLocation()))
							{
								TargetPlayer = Player;
							}
						}

						i++;
						FindPlayer();
					}

				}
			}

			if (DetectedItem)
			{
				if (Inventory->CurItemCount < 3) {
					if (GetWorld()->LineTraceSingleByChannel(OutHit, FollowCamera->GetComponentLocation() + FollowCamera->GetForwardVector()*200.0f, DetectedItem->GetActorLocation(), ECC_Visibility, CollisionParams))
					{
						break;
					}
					else
					{
						if (!IsMoveToItem && CurState == Neutral && isInside)
						{
							IsMoveToItem = true;
							FindItem(OverlapResult.GetActor());
						}
					}
				}
			}


		}
	}

}


float AAICharacter::Distance(FVector Vec1, FVector Vec2)
{
		return FMath::Sqrt(FMath::Pow((Vec1.X - Vec2.X), 2) + FMath::Pow((Vec1.Y - Vec2.Y), 2));
}

bool AAICharacter::CheckLocation(FVector Pos)
{
	return Distance(Pos, GetActorLocation()) < 300.0f;
}


void AAICharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	AICon = Cast<AProjectOneAIController>(GetController());


}

AAICharacter::AAICharacter()
{

	AIControllerClass = AProjectOneAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	GetCapsuleComponent()->InitCapsuleSize(25.0f, 30.0f);

	APAnim = Cast<UPlayerCharacterAnimInstance>(GetMesh()->GetAnimInstance());

	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...   
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;



	GetMesh()->SetWorldRotation(FRotator(0.0f, 270.0f, 0.0f));
	GetMesh()->SetWorldLocation(FVector(0.0f, 0.0, -97.0f));


	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	FollowCamera->SetRelativeLocation(FVector(-50.0f, 0, 0));


	CameraBoom->TargetArmLength = 350.0f;
	CameraBoom->SocketOffset = FVector(40.0f, 50.0f, 60.0f);


	CharacterStat->CurHP = 200.0f;
	HPBarWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBarWidgetComponent"));

	HPBarWidgetComponent->SetupAttachment(GetCapsuleComponent());

	HPBarWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 180.0f));
	HPBarWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);

	DetectedRange = 2000.0f;

	static ConstructorHelpers::FClassFinder<UUserWidget> HPWIDGET(TEXT("WidgetBlueprint'/Game/ProjectOneBlueprint/UI_EnemyHPBar.UI_EnemyHPBar_C'"));
	if (HPWIDGET.Succeeded())
	{
		HPBarWidgetComponent->SetWidgetClass(HPWIDGET.Class);
		HPBarWidgetComponent->SetupAttachment(RootComponent);
		HPBarWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 70.0f));
		HPBarWidgetComponent->SetDrawSize(FVector2D(200.0f, 10.0f));
	}

	SetResources();
}

void AAICharacter::Aim()
{
	if (!isAim) {
		isAim = true;
		GetCharacterMovement()->bUseControllerDesiredRotation = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;
	}
	else {
		isAim = false;

		GetCharacterMovement()->bUseControllerDesiredRotation = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}
}