// Fill out your copyright notice in the Description page of Project Settings.


#include "NumberCell.h"


int ANumberCell::GetNumber() const {
	return mNumber;
}

void ANumberCell::SetNumber(int n) {
	if (1 <= n && n <= 4) {
		mNumber = n;
		SetMeshComponent();
		return;
	}
	UE_LOG(LogTemp, Error, TEXT("LpSolver::SetNumber() unexpected number: %d"), n);
}

void ANumberCell::SetMeshComponent() {
	switch (mNumber) {
	case 1:
		mCellMesh->SetStaticMesh(mBlack1);
		break;
	case 2:
		mCellMesh->SetStaticMesh(mBlack2);
		break;
	case 3:
		mCellMesh->SetStaticMesh(mBlack3);
		break;
	case 4:
		mCellMesh->SetStaticMesh(mBlack4);
		break;
	}
}

