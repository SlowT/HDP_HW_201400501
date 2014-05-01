#pragma once

class SkinnedMesh;

class CollisionBox {
public:
	enum AXIS {
		AXIS_X = 0,
		AXIS_Y = 1,
		AXIS_Z = 2,
		AXIS_NUM = 3,
	};

	CollisionBox();
	CollisionBox( const D3DXVECTOR3& fMax, const D3DXVECTOR3& fMin );
	virtual ~CollisionBox();

	static std::shared_ptr<CollisionBox> Create( LPDIRECT3DDEVICE9 d3dDevice );

	bool			Init( LPDIRECT3DDEVICE9 d3dDevice );

	virtual void	Render();
	virtual void	Update( std::chrono::duration<float> dTime );

	bool			CollisionCheck( std::shared_ptr<CollisionBox> target ){
		return mObbCheck ? CollisionCheckOBB( target ) : CollisionCheckAABB( target );
	}
	bool			CollisionCheckAABB( std::shared_ptr<CollisionBox> target );
	bool			CollisionCheckOBB( std::shared_ptr<CollisionBox> target );

	void			move( D3DXVECTOR3 mVec ){	mPosition += mVec;	}

	void			SetPosition( D3DXVECTOR3 pos ){	mPosition = pos;	}
	void			SetFrontVec( D3DXVECTOR3 fVec ){ mFrontVector = fVec; }
	void			SetAxisLen( float xLen, float yLen, float zLen ){
		mAxisLen[AXIS_X] = xLen;
		mAxisLen[AXIS_Y] = yLen;
		mAxisLen[AXIS_Z] = zLen;
	}
	void			SetSpeed( D3DXVECTOR3 speedVec ){	mSpeed = speedVec;	}
	void			SetOBBCheck( bool obbCheck ){ mObbCheck = obbCheck; }
	void			SetColor( D3DXCOLOR color ){ mColor = color; }

	float			GetAxisLenX() { return mAxisLen[AXIS_X]; }
	float			GetAxisLenY() { return mAxisLen[AXIS_Y]; }
	float			GetAxisLenZ() { return mAxisLen[AXIS_Z]; }

private:
	float			mAxisLen[AXIS_NUM];
	LPDIRECT3DDEVICE9	mDevice;
	bool			mObbCheck = false;

	D3DXCOLOR		mColor = { 0.0f, 0.5f, 1.0f, 1.0f };
	D3DXVECTOR3		mPosition = { 0.f, 0.f, 0.f };
	D3DXVECTOR3		mScaleVec = { 1.f, 1.f, 1.f };
	D3DXVECTOR3		mFrontVector = { 0.f, 0.f, 1.f };
	D3DXVECTOR3		mUpVec = { 0.f, 1.f, 0.f };
	D3DXVECTOR3		mSpeed = { 0.f, 0.f, 0.f };
};
