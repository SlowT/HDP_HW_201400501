#include "stdafx.h"
#include <windows.h>
#include <d3d9.h>
#include "CollisionBox.h"

HINSTANCE	mHInstance;
HWND		mHwnd;
bool		keys[256] = { FALSE, };
LPDIRECT3DDEVICE9 mD3DDevice = nullptr;
std::vector<std::shared_ptr<CollisionBox>> mCollisionBoxList;
const D3DXVECTOR3 cameraPos = { 2.f, 3.f, -10.f };
const D3DXVECTOR3 cameraLookAt = { 2.f, 0.f, 0.f };
const D3DXVECTOR3 cameraUpVec = { 0.f, 1.f, 0.f };

LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
	case WM_DESTROY:{
		PostQuitMessage( 0 );
		return 0;
	}
	case WM_KEYDOWN:
	{
		keys[wParam] = TRUE;
		return 0;
	}

	case WM_KEYUP:
	{
		keys[wParam] = FALSE;
		return 0;
	}
	}

	return(DefWindowProc( hWnd, message, wParam, lParam ));
}

bool CreateWindowFrame( wchar_t* title, int width, int height ) {
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = NULL;
	wcex.cbWndExtra = NULL;
	wcex.hInstance = mHInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = title;
	wcex.hIconSm = NULL;
	wcex.hIcon = NULL;

	RegisterClassEx( &wcex );

	DWORD style = WS_OVERLAPPEDWINDOW;

	RECT wr = { 0, 0, width, height };
	AdjustWindowRect( &wr, style, FALSE );

	mHwnd = CreateWindow( title, title, style, CW_USEDEFAULT, CW_USEDEFAULT,
		wr.right - wr.left, wr.bottom - wr.top, NULL, NULL, wcex.hInstance, NULL );

	ShowWindow( mHwnd, SW_SHOWNORMAL );

	return true;
}

void Update( std::chrono::duration<float> dTime )
{
	for( auto box : mCollisionBoxList ){
		box->Update( dTime );
	}
}

void Render()
{
	mD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB( 255, 0, 0, 0 ), 1.0f, 0 );
	mD3DDevice->BeginScene();


	for( auto box : mCollisionBoxList ){
		box->Render();
	}

	mD3DDevice->EndScene();
	mD3DDevice->Present( NULL, NULL, NULL, NULL );
}

int _tmain(int argc, _TCHAR* argv[])
{
	MSG msg;

	if( CreateWindowFrame( L"Collsion Test", 800, 600 ) == false )
		return -1;


	D3DPRESENT_PARAMETERS d3dPresentParameters;
	LPDIRECT3D9 mD3D = Direct3DCreate9( D3D_SDK_VERSION );
	ZeroMemory( &d3dPresentParameters, sizeof(d3dPresentParameters) );
	D3DMULTISAMPLE_TYPE mst = D3DMULTISAMPLE_NONE;

	d3dPresentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dPresentParameters.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	d3dPresentParameters.BackBufferWidth = 800;
	d3dPresentParameters.BackBufferHeight = 600;
	d3dPresentParameters.BackBufferFormat = D3DFMT_A8R8G8B8;
	d3dPresentParameters.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	d3dPresentParameters.hDeviceWindow = mHwnd;
	d3dPresentParameters.Windowed = TRUE;
	d3dPresentParameters.EnableAutoDepthStencil = TRUE;
	d3dPresentParameters.AutoDepthStencilFormat = D3DFMT_D16;

	if( SUCCEEDED( mD3D->CheckDeviceMultiSampleType( D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL, d3dPresentParameters.BackBufferFormat, TRUE, mst, NULL ) ) ) {
		d3dPresentParameters.MultiSampleType = mst;
	}
	else {
		return false;
	}

	if( FAILED( mD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, mHwnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dPresentParameters, &mD3DDevice ) ) ) {
		return false;
	}

	mD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
	mD3DDevice->SetRenderState( D3DRS_ZENABLE, TRUE );

	D3DXMATRIXA16	MatWorld;
	D3DXMatrixIdentity( &MatWorld );
	mD3DDevice->SetTransform( D3DTS_WORLD, &MatWorld );

	D3DXMATRIXA16	MatView;
	D3DXMatrixLookAtLH( &MatView, &cameraPos, &cameraLookAt, &cameraUpVec );
	mD3DDevice->SetTransform( D3DTS_VIEW, &MatView );

	D3DXMATRIXA16	MatProj;
	D3DXMatrixPerspectiveFovLH( &MatProj, D3DXToRadian( 60.f ), 1.0f, 1.0f, 1000.0f );
	mD3DDevice->SetTransform( D3DTS_PROJECTION, &MatProj );


	mCollisionBoxList.push_back(CollisionBox::Create( mD3DDevice ));
	mCollisionBoxList[0]->move( D3DXVECTOR3( 0.f, 0.f, 0.f ) );
	mCollisionBoxList.push_back( CollisionBox::Create( mD3DDevice ) );
	mCollisionBoxList[1]->move( D3DXVECTOR3( 5.f, 0.f, 0.f ) );
	mCollisionBoxList[1]->SetSpeed( D3DXVECTOR3( -0.1f, 0.f, 0.f ) );
	mCollisionBoxList.push_back( CollisionBox::Create( mD3DDevice ) );
	mCollisionBoxList[2]->move( D3DXVECTOR3( 0.f, 0.f, 5.f ) );
	mCollisionBoxList[2]->SetFrontVec( D3DXVECTOR3( -1.f, 0.f, 2.f ) );
	mCollisionBoxList[2]->SetOBBCheck( true );
	mCollisionBoxList.push_back( CollisionBox::Create( mD3DDevice ) );
	mCollisionBoxList[3]->move( D3DXVECTOR3( 5.f, 0.f, 5.5f ) );
	mCollisionBoxList[3]->SetSpeed( D3DXVECTOR3( -0.1f, 0.f, 0.f ) );
	mCollisionBoxList[3]->SetOBBCheck( true );

	std::chrono::system_clock::time_point prev( std::chrono::system_clock::now() );
	PeekMessage( &msg, NULL, 0, 0, PM_REMOVE );
	while( msg.message != WM_QUIT ) {
		TranslateMessage( &msg );
		DispatchMessage( &msg );

		Update( std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::system_clock::now() - prev) );
		
		for( auto pTarget : mCollisionBoxList ) {
			for( auto box : mCollisionBoxList ) {
 				if( box->CollisionCheck( pTarget ) ) {
					box->SetColor( D3DXCOLOR( 1.f, 0.f, 0.f, 1.f ) );
					box->SetSpeed( D3DXVECTOR3( 0.f, 0.f, 0.f ) );
 				}
			}
		}

		Render();

		PeekMessage( &msg, NULL, 0, 0, PM_REMOVE );
	}

	return 0;
}

