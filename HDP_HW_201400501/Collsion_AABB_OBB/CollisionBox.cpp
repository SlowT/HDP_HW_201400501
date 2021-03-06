﻿#include "stdafx.h"
#include "CollisionBox.h"

CollisionBox::CollisionBox() {
}

CollisionBox::~CollisionBox() {
}

std::shared_ptr<CollisionBox> CollisionBox::Create( LPDIRECT3DDEVICE9 d3dDevice ) {
	std::shared_ptr<CollisionBox> pCollisionBox(new CollisionBox);

	if( pCollisionBox->Init( d3dDevice ) ) {
		return pCollisionBox;
	}
	else 
		return nullptr;
}

bool CollisionBox::Init( LPDIRECT3DDEVICE9 d3dDevice ) {
	mDevice = d3dDevice;

	mAxisLen[AXIS_X] = 0.5f;
	mAxisLen[AXIS_Y] = 0.5f;
	mAxisLen[AXIS_Z] = 0.5f;

	return true;
}

void CollisionBox::Render()
{
	D3DXMATRIX projMat, viewMat;
	mDevice->GetTransform( D3DTS_PROJECTION, &projMat );
	mDevice->GetTransform( D3DTS_VIEW, &viewMat );
	viewMat *= projMat;

	ID3DXLine *Line;

	if( D3DXCreateLine( mDevice, &Line ) != D3D_OK )
		return;
	Line->SetWidth( 1 );
	Line->SetAntialias( true );

	D3DXVECTOR3 mXDirVec, tXDirVec;
	D3DXVec3Normalize( &mFrontVector, &mFrontVector );
	D3DXVec3Cross( &mXDirVec, &mUpVec, &mFrontVector );
	D3DXVECTOR3 mAxisDir[3] = { mXDirVec, mUpVec, mFrontVector };
	D3DXVECTOR3 vF[3];
	for( int i = 0; i < 3; ++i ){
		vF[i] = mAxisDir[i] * mAxisLen[i];
	}

	D3DXVECTOR3 point[10];
	point[0] = mPosition - vF[0] + vF[1] + vF[2];
	point[1] = mPosition + vF[0] + vF[1] + vF[2];
	point[2] = mPosition + vF[0] + vF[1] - vF[2];
	point[3] = mPosition - vF[0] + vF[1] - vF[2];
	point[4] = mPosition - vF[0] + vF[1] + vF[2];
	point[5] = mPosition - vF[0] - vF[1] + vF[2];
	point[6] = mPosition + vF[0] - vF[1] + vF[2];
	point[7] = mPosition + vF[0] - vF[1] - vF[2];
	point[8] = mPosition - vF[0] - vF[1] - vF[2];
	point[9] = mPosition - vF[0] - vF[1] + vF[2];
	Line->Begin();
	Line->DrawTransform( point, 10, &viewMat, mColor );

	D3DXVECTOR3 pointT[2];
	pointT[0] = mPosition - vF[0] - vF[1] - vF[2];
	pointT[1] = mPosition - vF[0] + vF[1] - vF[2];
	Line->DrawTransform( pointT, 2, &viewMat, mColor );

	pointT[0] = mPosition + vF[0] - vF[1] - vF[2];
	pointT[1] = mPosition + vF[0] + vF[1] - vF[2];
	Line->DrawTransform( pointT, 2, &viewMat, mColor );

	pointT[0] = mPosition + vF[0] - vF[1] + vF[2];
	pointT[1] = mPosition + vF[0] + vF[1] + vF[2];
	Line->DrawTransform( pointT, 2, &viewMat, mColor );
	Line->End();

	Line->Release();
}

void CollisionBox::Update( std::chrono::duration<float> dTime ) {
	move( mSpeed * dTime.count() );
}

bool CollisionBox::CollisionCheckAABB( std::shared_ptr<CollisionBox> target ) {
	if( this == target.get() )
		return false;

	D3DXVECTOR3 tPos = target->mPosition;
	for( int idx = 0; idx < 3; ++idx ){
		if( std::abs( tPos[idx] - mPosition[idx] ) > mAxisLen[idx] + target->mAxisLen[idx] )
			return false;
	}
	return true;
}

bool CollisionBox::CollisionCheckOBB( std::shared_ptr<CollisionBox> target ) {
	D3DXVECTOR3 D = mPosition - target->mPosition;

	if( this == target.get() )
		return false;

	//Start Check By OBB
	float C[3][3];
	float absC[3][3];
	float AD[3];
	float R0, R1, R;
	float R01;
	D3DXVECTOR3 mXDirVec, tXDirVec, fVec;
	D3DXVec3Normalize( &fVec, &mFrontVector );
	D3DXVec3Cross( &mXDirVec, &mUpVec, &fVec );
	D3DXVECTOR3 mAxisDir[3] = { mXDirVec, mUpVec, fVec };
	D3DXVec3Normalize( &fVec, &target->mFrontVector );
	D3DXVec3Cross( &tXDirVec, &target->mUpVec, &fVec );
	D3DXVECTOR3 tAxisDir[3] = { tXDirVec, target->mUpVec, fVec };

	//A0~2
	for( int j = 0; j < 3; ++j ){
		for( int i = 0; i < 3; ++i ){
			C[j][i] = D3DXVec3Dot( &tAxisDir[j], &mAxisDir[i] );
			absC[j][i] = abs( C[j][i] );
		}
		AD[j] = D3DXVec3Dot( &tAxisDir[j], &D );
		R = abs( AD[j] );
		R1 = mAxisLen[0] * absC[j][0] + mAxisLen[1] * absC[j][1] + mAxisLen[2] * absC[j][2];
		R01 = target->mAxisLen[j] + R1;
		if( R > R01 ) return false;
	}

	//B0~2
	for( int j = 0; j < 3; ++j ){
		R = abs( D3DXVec3Dot( &mAxisDir[j], &D ) );
		R0 = target->mAxisLen[0] * absC[0][j] + target->mAxisLen[1] * absC[1][j] + target->mAxisLen[2] * absC[2][j];
		R01 = R0 + mAxisLen[j];
		if( R > R01 ) return false;
	}

	//A0xB0
	R = abs( AD[2] * C[1][0] - AD[1] * C[2][0] );
	R0 = target->mAxisLen[1] * absC[2][0] + target->mAxisLen[2] * absC[1][0];
	R1 = mAxisLen[1] * absC[0][2] + mAxisLen[2] * absC[0][1];
	R01 = R0 + R1;
	if( R > R01 ) return false;

	//A0xB1
	R = abs( AD[2] * C[1][1] - AD[1] * C[2][1] );
	R0 = target->mAxisLen[1] * absC[2][1] + target->mAxisLen[2] * absC[1][1];
	R1 = mAxisLen[0] * absC[0][2] + mAxisLen[2] * absC[0][0];
	R01 = R0 + R1;
	if( R > R01 ) return false;

	//A0xB2
	R = abs( AD[2] * C[1][2] - AD[1] * C[2][2] );
	R0 = target->mAxisLen[1] * absC[2][2] + target->mAxisLen[2] * absC[1][2];
	R1 = mAxisLen[0] * absC[0][1] + mAxisLen[1] * absC[0][0];
	R01 = R0 + R1;
	if( R > R01 )return false;

	//A1xB0
	R = abs( AD[0] * C[2][0] - AD[2] * C[0][0] );
	R0 = target->mAxisLen[0] * absC[2][0] + target->mAxisLen[2] * absC[0][0];
	R1 = mAxisLen[1] * absC[1][2] + mAxisLen[2] * absC[1][1];
	R01 = R0 + R1;
	if( R > R01 ) return false;

	//A1xB1
	R = abs( AD[0] * C[2][1] - AD[2] * C[0][1] );
	R0 = target->mAxisLen[0] * absC[2][1] + target->mAxisLen[2] * absC[0][1];
	R1 = mAxisLen[0] * absC[1][2] + mAxisLen[2] * absC[1][0];
	R01 = R0 + R1;
	if( R > R01 ) return false;

	//A1xB2
	R = abs( AD[0] * C[2][2] - AD[2] * C[0][2] );
	R0 = target->mAxisLen[0] * absC[2][2] + target->mAxisLen[2] * absC[0][2];
	R1 = mAxisLen[0] * absC[1][1] + mAxisLen[1] * absC[1][0];
	R01 = R0 + R1;
	if( R > R01 ) return false;

	//A2xB0
	R = abs( AD[1] * C[0][0] - AD[0] * C[1][0] );
	R0 = target->mAxisLen[0] * absC[1][0] + target->mAxisLen[1] * absC[0][0];
	R1 = mAxisLen[1] * absC[2][2] + mAxisLen[2] * absC[2][1];
	R01 = R0 + R1;
	if( R > R01 ) return false;

	//A2xB1
	R = abs( AD[1] * C[0][1] - AD[0] * C[1][1] );
	R0 = target->mAxisLen[0] * absC[1][1] + target->mAxisLen[1] * absC[0][1];
	R1 = mAxisLen[0] * absC[2][2] + mAxisLen[2] * absC[2][0];
	R01 = R0 + R1;
	if( R > R01 ) return false;

	//A2xB2
	R = abs( AD[1] * C[0][2] - AD[0] * C[1][2] );
	R0 = target->mAxisLen[0] * absC[1][2] + target->mAxisLen[1] * absC[0][2];
	R1 = mAxisLen[0] * absC[2][1] + mAxisLen[1] * absC[2][0];
	R01 = R0 + R1;
	if( R > R01 ) return false;

	return true;
}
