// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ProjectOne.h"
#include "Player/ProjectOneCharacter.h"
#include "Environments/EQS/EQSVariables.h"
#include "Cover.h"
#include "AICharacter.generated.h"


enum AIState
{
	Neutral,
	Offensive,
	Diffensive,
	Guard,
	Dead
};

enum AttackPattern
{
	Pattern1,
	Pattern2,
	Pattern3,
	Pattern4
};

/**
 * 
 */
UCLASS()
class PROJECTONE_API AAICharacter : public AProjectOneCharacter
{
	GENERATED_BODY()


public:
	AAICharacter();

	//void SetResources() override;

	virtual void PostInitializeComponents() override;
	virtual void Tick(float delta) override;
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	class AProjectOneAIController * AICon;
	class AEQSSystem * EQSSystem;
	
	//내비를 안쓴 이동함수들
	void Forward();
	void Back();
	void Left();
	void RIght();

	//해당 벡터로 Controller를 회전
	void LookAt(FVector LookPos);

	//탐지 함수
	void Detected();

	// 2차원 거리 함수
	float Distance(FVector Vec1, FVector Vec2);

	//현재 타겟 캐릭터
	AProjectOneCharacter * TargetPlayer;

	//목표점
	FVector TargetLocationPos;

	//목표점에 도착했는지 판단
	bool CheckLocation(FVector Pos);

	//탐지한 모든 캐릭터들
	TArray<AProjectOneCharacter*> DetectedPlayers;

	bool RigthCheck; //오른쪽 벡터에 장애물이 있는지
	bool LeftCheck; // 왼쪽 벡터에 장애물이 있는지
	bool IsMoving; //움직이고 있는지

	FTimerHandle  Delay; //딜레이용 핸들러
	FTimerHandle InitTime; // 초기화용 핸들러
	FTimerHandle MagneticFieldTimer; //자기장용
	FTimerHandle ItemTimer; //아이템용
	FTimerHandle GuardTimerHandle; //경계용
	FTimerHandle DetectedTimer; //탐지용


	bool IsLookAround; //현재 경계중인지
	bool LeftEnd; //경계 처리용 bool
	float LeftAngle; //경계 목표 각도
	float RightAngle;//경계 목표 오른쪽 각도
	int LookCount; //경계 왕복 카운트

	void LookAround(); //경계 함수
	void LookAroundStart(); //경계 용 변수들 초기화

	virtual void Shooting(float tick) override; 

	//플레이어 움직임 예측함수
	FVector Prediction();

	//AI 현재 상태
	AIState CurState;

	void Update();

	//내비를 쓴 이동함수
	void MoveToLocation(FVector Location);

	//숨기
	void Hide(AActor * Attacker);
	bool Hiding;
	FVector HidePos;

	//마지막 공격자
	AActor * LastAttaker;



	bool MoveEnd; //움직임 제어용 bool


	void Attacked(AActor * Attacker);  //공격받았을때 판단함수
	void FindPlayer(); //캐릭터를 발견했을때 판단


	void InitTargetPlayer(); //타겟 플레이어 초기화


	//06-11 이후 변수 및 함수
	UPROPERTY()
	TArray<FEQSItem> Path; //Astar 용 path

	int32 pathCount; // Astar 조절할 변수

	void AstarMove(FVector MovePoint); //Astar 이동

	//자기장
	class AProjectOneGameState * MagneticField;
	
	FEQSItem GetBestPos(FVector SearchCenter, int SearchSize); //탐지범위에서 가장 좋은 점을 반환;
	 
	void MovingInside();

	FVector TargetPos; //목표지점

	TArray<FVector> PastPos; //지났던 pos들;

	FVector GetInsidePos(); //자기장 안쪽 pos를 가져오는 함수
	FVector GetInsideNearestPos(); //자지장 안쪽 가장 가까운 pos를 가져오는 함수

	bool isFail; //path 탐색중 실패할경우 함수 탈충용 불타입

	bool isDelay; //경계 딜레이 중인지
	float DelayTime; // 경계 딜레이 시간

	void DelayTimer(); //경계 딜레이 타이머

	void FindItem(AActor * DeitemPos); //item을 발견했을때 판단

	bool IsMoveToItem; //아이템으로 이동중인지
	void AllStop(); //모든동작 중지 및 변수 초기화

	void SetTargetPos(); //목표 지점을 정하는 함수

	void CheckMagneticField(); //자기장 체크함수

	bool isInside; //자기장 안쪽인지

	void ItemCheck(); //아이템을 사용할지 체크

	float CurrentMagneticFieldRadius; //자기장의 현재 반지름

	bool IsPattern; //패턴중인지
	bool IsPatternDelay; //패턴 대기 시간
	AttackPattern CurPattern; //현재 공격 패턴

	void Pattern();//패턴행동
	void SetPattern();//패턴생성
	int DelayCount; //패턴조절용 딜레이 카운트

	void PatternTimer(); // 0.1초용 타이머

	bool isGuard; //경계중인지
	float GuardTime; //경계 시간
	void SetGuardPattern(); //경계 패턴 결정

	void GuardTimer(); //경계 딜레이 체크

	bool IsMoveToLastAttaker; //마지막 공격자에게 도달했는지
	FVector LastAttakerPos; //마지막 공격자의 마지막  위치

	bool IsFinal; //자기장이 마지막인짖

	float DetectedRange; //탐지 범위


	float StartTime;

	void Aim();
};