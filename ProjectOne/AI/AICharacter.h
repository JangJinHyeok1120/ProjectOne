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
	
	//���� �Ⱦ� �̵��Լ���
	void Forward();
	void Back();
	void Left();
	void RIght();

	//�ش� ���ͷ� Controller�� ȸ��
	void LookAt(FVector LookPos);

	//Ž�� �Լ�
	void Detected();

	// 2���� �Ÿ� �Լ�
	float Distance(FVector Vec1, FVector Vec2);

	//���� Ÿ�� ĳ����
	AProjectOneCharacter * TargetPlayer;

	//��ǥ��
	FVector TargetLocationPos;

	//��ǥ���� �����ߴ��� �Ǵ�
	bool CheckLocation(FVector Pos);

	//Ž���� ��� ĳ���͵�
	TArray<AProjectOneCharacter*> DetectedPlayers;

	bool RigthCheck; //������ ���Ϳ� ��ֹ��� �ִ���
	bool LeftCheck; // ���� ���Ϳ� ��ֹ��� �ִ���
	bool IsMoving; //�����̰� �ִ���

	FTimerHandle  Delay; //�����̿� �ڵ鷯
	FTimerHandle InitTime; // �ʱ�ȭ�� �ڵ鷯
	FTimerHandle MagneticFieldTimer; //�ڱ����
	FTimerHandle ItemTimer; //�����ۿ�
	FTimerHandle GuardTimerHandle; //����
	FTimerHandle DetectedTimer; //Ž����


	bool IsLookAround; //���� ���������
	bool LeftEnd; //��� ó���� bool
	float LeftAngle; //��� ��ǥ ����
	float RightAngle;//��� ��ǥ ������ ����
	int LookCount; //��� �պ� ī��Ʈ

	void LookAround(); //��� �Լ�
	void LookAroundStart(); //��� �� ������ �ʱ�ȭ

	virtual void Shooting(float tick) override; 

	//�÷��̾� ������ �����Լ�
	FVector Prediction();

	//AI ���� ����
	AIState CurState;

	void Update();

	//���� �� �̵��Լ�
	void MoveToLocation(FVector Location);

	//����
	void Hide(AActor * Attacker);
	bool Hiding;
	FVector HidePos;

	//������ ������
	AActor * LastAttaker;



	bool MoveEnd; //������ ����� bool


	void Attacked(AActor * Attacker);  //���ݹ޾����� �Ǵ��Լ�
	void FindPlayer(); //ĳ���͸� �߰������� �Ǵ�


	void InitTargetPlayer(); //Ÿ�� �÷��̾� �ʱ�ȭ


	//06-11 ���� ���� �� �Լ�
	UPROPERTY()
	TArray<FEQSItem> Path; //Astar �� path

	int32 pathCount; // Astar ������ ����

	void AstarMove(FVector MovePoint); //Astar �̵�

	//�ڱ���
	class AProjectOneGameState * MagneticField;
	
	FEQSItem GetBestPos(FVector SearchCenter, int SearchSize); //Ž���������� ���� ���� ���� ��ȯ;
	 
	void MovingInside();

	FVector TargetPos; //��ǥ����

	TArray<FVector> PastPos; //������ pos��;

	FVector GetInsidePos(); //�ڱ��� ���� pos�� �������� �Լ�
	FVector GetInsideNearestPos(); //������ ���� ���� ����� pos�� �������� �Լ�

	bool isFail; //path Ž���� �����Ұ�� �Լ� Ż��� ��Ÿ��

	bool isDelay; //��� ������ ������
	float DelayTime; // ��� ������ �ð�

	void DelayTimer(); //��� ������ Ÿ�̸�

	void FindItem(AActor * DeitemPos); //item�� �߰������� �Ǵ�

	bool IsMoveToItem; //���������� �̵�������
	void AllStop(); //��絿�� ���� �� ���� �ʱ�ȭ

	void SetTargetPos(); //��ǥ ������ ���ϴ� �Լ�

	void CheckMagneticField(); //�ڱ��� üũ�Լ�

	bool isInside; //�ڱ��� ��������

	void ItemCheck(); //�������� ������� üũ

	float CurrentMagneticFieldRadius; //�ڱ����� ���� ������

	bool IsPattern; //����������
	bool IsPatternDelay; //���� ��� �ð�
	AttackPattern CurPattern; //���� ���� ����

	void Pattern();//�����ൿ
	void SetPattern();//���ϻ���
	int DelayCount; //���������� ������ ī��Ʈ

	void PatternTimer(); // 0.1�ʿ� Ÿ�̸�

	bool isGuard; //���������
	float GuardTime; //��� �ð�
	void SetGuardPattern(); //��� ���� ����

	void GuardTimer(); //��� ������ üũ

	bool IsMoveToLastAttaker; //������ �����ڿ��� �����ߴ���
	FVector LastAttakerPos; //������ �������� ������  ��ġ

	bool IsFinal; //�ڱ����� ��������¢

	float DetectedRange; //Ž�� ����


	float StartTime;

	void Aim();
};