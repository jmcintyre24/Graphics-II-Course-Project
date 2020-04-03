#pragma once
#include "defines.h"

class DrawClass
{
public:
	DrawClass(GW::GRAPHICS::GDirectX11Surface _d3d11, GW::SYSTEM::GWindow _win);
	static HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
	{
		HRESULT hr = S_OK;

		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
		// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
		// Setting this flag improves the shader debugging experience, but still allows 
		// the shaders to be optimized and to run exactly the way they will run in 
		// the release configuration of this program.
		dwShaderFlags |= D3DCOMPILE_DEBUG;

		// Disable optimizations to further improve shader debugging
		dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		ID3DBlob* pErrorBlob = nullptr;
		hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
			dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
		if (FAILED(hr))
		{
			if (pErrorBlob)
			{
				OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
				pErrorBlob->Release();
			}
			return hr;
		}
		if (pErrorBlob) pErrorBlob->Release();

		return S_OK;
	}
protected:
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GDirectX11Surface d3d11;
	unsigned int width = 0, height = 0;
private:

};

class Triangle : DrawClass
{
	struct SimpleVertex
	{
		XMFLOAT3 Pos;
	};

	Microsoft::WRL::ComPtr<ID3D11InputLayout>	input = nullptr;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	vertexshader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelshader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		vertexbuffer = nullptr;
public:
	Triangle(GW::GRAPHICS::GDirectX11Surface _d3d11, GW::SYSTEM::GWindow _win) : DrawClass(_d3d11, _win)
	{
		d3d11 = _d3d11;
		win = _win;
		ID3D11Device* dev = nullptr;
		ID3D11DeviceContext* con = nullptr;
		+d3d11.GetDevice((void**)&dev);
		+d3d11.GetImmediateContext((void**)(&con));

		// Compile the vertex shader
		Microsoft::WRL::ComPtr<ID3DBlob> errors;
		ID3DBlob* pVSBlob = nullptr;
		if (FAILED(DrawClass::CompileShaderFromFile(L"shaders.fx", "VS", "vs_4_0", &pVSBlob)))
		{
			std::cout << (char*)errors->GetBufferPointer() << std::endl;
			return;
		}

		errors.Reset();
		// Create the vertex shader
		if (FAILED(dev->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, vertexshader.GetAddressOf())))
		{
			std::cout << (char*)errors->GetBufferPointer() << std::endl;
			pVSBlob->Release();
			return;
		}

		// Define the input layout
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		UINT numElements = ARRAYSIZE(layout);

		// Create the input layout
		if (FAILED(dev->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), input.GetAddressOf())))
		{
			pVSBlob->Release();
			return;
		}
		pVSBlob->Release();
		// Compile the pixel shader
		ID3DBlob* pPSBlob = nullptr;
		if (FAILED(DrawClass::CompileShaderFromFile(L"shaders.fx", "PS", "ps_4_0", &pPSBlob)))
		{
			MessageBox(nullptr,
				L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
			return;
		}

		// Create the pixel shader
		if (FAILED(dev->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, pixelshader.GetAddressOf())))
		{
			std::cout << (char*)errors->GetBufferPointer() << std::endl;
			pPSBlob->Release();
			return;
		}
		pPSBlob->Release();
		// Create vertex buffer
		SimpleVertex vertices[] =
		{
			XMFLOAT3(0.0f, 1.0f, 0.5f),
			XMFLOAT3(1.0f, -1.0f, 0.5f),
			XMFLOAT3(-1.0f, -1.0f, 0.5f),
		};

		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(SimpleVertex) * 3;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA InitData = {};
		InitData.pSysMem = vertices;
		if (FAILED(dev->CreateBuffer(&bd, &InitData, vertexbuffer.GetAddressOf())))
			return;

		con->Release();
		dev->Release();
		return;
	}

	void Render()
	{
		// Render a triangle
		ID3D11DeviceContext* con;
		ID3D11RenderTargetView* view;
		d3d11.GetImmediateContext((void**)&con);
		d3d11.GetRenderTargetView((void**)&view);
		// setup the pipeline
		ID3D11RenderTargetView* const views[] = { view };
		con->OMSetRenderTargets(ARRAYSIZE(views), views, nullptr);
		// Set vertex buffer
		const UINT stride[] = { sizeof(SimpleVertex) };
		const UINT offset[] = { 0 };
		ID3D11Buffer* const buffs[] = { vertexbuffer.Get() };
		con->IASetVertexBuffers(0, ARRAYSIZE(buffs), buffs, stride, offset);

		con->VSSetShader(vertexshader.Get(), nullptr, 0);
		con->PSSetShader(pixelshader.Get(), nullptr, 0);
		con->IASetInputLayout(input.Get());
		con->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		con->Draw(3, 0);

		con->Release();
		view->Release();
	}
private:
};

class Cube : DrawClass
{
	struct ConstantBuffer
	{
		XMMATRIX mWorld;
		XMMATRIX mView;
		XMMATRIX mProjection;
	};

	Microsoft::WRL::ComPtr<ID3D11InputLayout>	input = nullptr;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	vertexshader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelshader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		vertexbuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		indexbuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		constantbuffer = nullptr;
	XMMATRIX									g_World;
	XMMATRIX									g_View;
	XMMATRIX									g_Projection;

public:
	struct SimpleVertex
	{
		XMFLOAT3 Pos;
		XMFLOAT4 Color;
	};

	struct SimpleMesh
	{
		std::vector<SimpleVertex> vertexList;
		std::vector<unsigned int> indicesList;
	};

private: 	
	SimpleMesh* mesh = nullptr;

public:

	Cube(GW::GRAPHICS::GDirectX11Surface _d3d11, GW::SYSTEM::GWindow _win, SimpleMesh* _mesh = nullptr) : DrawClass(_d3d11, _win)
	{
		mesh = _mesh;
		ID3D11Device* dev = nullptr;
		ID3D11DeviceContext* con = nullptr;
		+d3d11.GetDevice((void**)&dev);
		+d3d11.GetImmediateContext((void**)(&con));

		// Compile the vertex shader
		Microsoft::WRL::ComPtr<ID3DBlob> errors;
		ID3DBlob* pVSBlob = nullptr;
		if (FAILED(DrawClass::CompileShaderFromFile(L"shaders.fx", "VS", "vs_4_0", &pVSBlob)))
		{
			std::cout << (char*)errors->GetBufferPointer() << std::endl;
			return;
		}

		errors.Reset();
		// Create the vertex shader
		if (FAILED(dev->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, vertexshader.GetAddressOf())))
		{
			std::cout << (char*)errors->GetBufferPointer() << std::endl;
			pVSBlob->Release();
			return;
		}

		// Define the input layout
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		UINT numElements = ARRAYSIZE(layout);

		// Create the input layout
		if (FAILED(dev->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), input.GetAddressOf())))
		{
			pVSBlob->Release();
			return;
		}
		pVSBlob->Release();
		// Compile the pixel shader
		ID3DBlob* pPSBlob = nullptr;
		if (FAILED(DrawClass::CompileShaderFromFile(L"shaders.fx", "PS", "ps_4_0", &pPSBlob)))
		{
			MessageBox(nullptr,
				L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
			return;
		}

		// Create the pixel shader
		if (FAILED(dev->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, pixelshader.GetAddressOf())))
		{
			std::cout << (char*)errors->GetBufferPointer() << std::endl;
			pPSBlob->Release();
			return;
		}
		pPSBlob->Release();
		

		D3D11_BUFFER_DESC bd = {};
		D3D11_SUBRESOURCE_DATA InitData = {};
		// If there's no mesh, render a normal cube.
		if (mesh == nullptr)
		{
			// Create vertex buffer
			SimpleVertex vertices[] =
			{
				{ XMFLOAT3(-1.0f, 1.0f, -1.0f),   XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
				{ XMFLOAT3(1.0f, 1.0f, -1.0f),    XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
				{ XMFLOAT3(1.0f, 1.0f, 1.0f),     XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
				{ XMFLOAT3(-1.0f, 1.0f, 1.0f),    XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },

				{ XMFLOAT3(-1.0f, -1.0f, -1.0f),  XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
				{ XMFLOAT3(1.0f, -1.0f, -1.0f),   XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
				{ XMFLOAT3(1.0f, -1.0f, 1.0f),    XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
				{ XMFLOAT3(-1.0f, -1.0f, 1.0f),   XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
			};

			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(SimpleVertex) * 8;
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.CPUAccessFlags = 0;

			InitData.pSysMem = vertices;
			if (FAILED(dev->CreateBuffer(&bd, &InitData, vertexbuffer.GetAddressOf())))
				return;
		}
		else
		{
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(SimpleVertex) * mesh->vertexList.size();
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.CPUAccessFlags = 0;

			InitData.pSysMem = mesh->vertexList.data();
			if (FAILED(dev->CreateBuffer(&bd, &InitData, vertexbuffer.GetAddressOf())))
				return;
		}

		if (mesh == nullptr)
		{
			// Create index buffer
			WORD indices[] =
			{
				3,1,0,
				2,1,3,

				0,5,4,
				1,5,0,

				3,4,7,
				0,4,3,

				1,6,5,
				2,6,1,

				2,7,6,
				3,7,2,

				6,4,5,
				7,4,6,
			};
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(WORD) * 36;        // 36 vertices needed for 12 triangles in a triangle list
			bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bd.CPUAccessFlags = 0;
			InitData.pSysMem = indices;
			if (FAILED(dev->CreateBuffer(&bd, &InitData, indexbuffer.GetAddressOf())))
				return;
		}
		else
		{
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(int) * mesh->indicesList.size();        // 36 vertices needed for 12 triangles in a triangle list
			bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bd.CPUAccessFlags = 0;
			InitData.pSysMem = mesh->indicesList.data();
			if (FAILED(dev->CreateBuffer(&bd, &InitData, indexbuffer.GetAddressOf())))
				return;
		}


		// Create the constant buffer
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(ConstantBuffer);
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		if (FAILED(dev->CreateBuffer(&bd, nullptr, constantbuffer.GetAddressOf())))
			return;

		// Initialize the world matrix
		g_World = XMMatrixIdentity();

		// Initialize the view matrix
		XMVECTOR Eye = XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f);
		XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		g_View = XMMatrixLookAtLH(Eye, At, Up);

		// Initialize the projection matrix
		g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, DrawClass::width / (FLOAT)DrawClass::height, 0.01f, 100.0f);

		con->Release();
		dev->Release();
		return;
	}

	void Render()
	{
		// Render a triangle
		ID3D11DeviceContext* con;
		ID3D11RenderTargetView* view;
		d3d11.GetImmediateContext((void**)&con);
		d3d11.GetRenderTargetView((void**)&view);
		// setup the pipeline
		ID3D11RenderTargetView* const views[] = { view };
		con->OMSetRenderTargets(ARRAYSIZE(views), views, nullptr);
		// Set vertex buffer
		const UINT stride[] = { sizeof(SimpleVertex) };
		const UINT offset[] = { 0 };
		ID3D11Buffer* const buffs[] = { vertexbuffer.Get() };
		con->IASetVertexBuffers(0, ARRAYSIZE(buffs), buffs, stride, offset);
		//Set Index Buffer
		if(mesh == nullptr)
			con->IASetIndexBuffer(indexbuffer.Get(), DXGI_FORMAT_R16_UINT, 0); //Set Index Buffer
		else
			con->IASetIndexBuffer(indexbuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		ConstantBuffer cb;
		cb.mWorld = XMMatrixTranspose(g_World);
		cb.mView = XMMatrixTranspose(g_View);
		cb.mProjection = XMMatrixTranspose(g_Projection);
		con->UpdateSubresource(constantbuffer.Get(), 0, nullptr, &cb, 0, 0);

		con->VSSetShader(vertexshader.Get(), nullptr, 0);
		con->VSSetConstantBuffers(0, 1, constantbuffer.GetAddressOf());
		con->PSSetShader(pixelshader.Get(), nullptr, 0);
		con->IASetInputLayout(input.Get());
		con->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		if (mesh == nullptr)
			con->DrawIndexed(36, 0, 0);        // 36 vertices needed for 12 triangles in a triangle list
		else
			con->DrawIndexed(mesh->indicesList.size(), 0, 0);

		con->Release();
		view->Release();
	}

	void UserInput()
	{
		//Rotate the objects.
		if (GetAsyncKeyState('J'))
		{
			g_World *= XMMatrixRotationY(0.1f);
		}

		if (GetAsyncKeyState('L'))
		{
			g_World *= XMMatrixRotationY(-0.1f);
		}

		//Look around movement
		if ((GetKeyState(VK_RBUTTON) & 0x100) != 0)
		{
			POINT cursorPos;
			GetCursorPos(&cursorPos);

			unsigned int clientPosX, clientPosY;
			win.GetX(clientPosX);
			win.GetY(clientPosY);
			unsigned int cosX = clientPosX + (width / 2);
			unsigned int cosY = clientPosY + (height / 2);

			int diffX = (cosX - cursorPos.x);
			int diffY = (cosY - cursorPos.y);

			if (diffX < -mouseThreshold)
			{
				g_View *= XMMatrixRotationY(-0.05f);
			}
			else if (diffX > mouseThreshold)
			{
				g_View *= XMMatrixRotationY(0.05f);
			}

			if (diffY < -mouseThreshold)
			{
				g_View *= XMMatrixRotationX(-0.05f);

			}
			else if (diffY > mouseThreshold)
			{
				g_View *= XMMatrixRotationX(0.05f);
			}

			//Set it back to the center
			SetCursorPos(cosX, cosY);
		}

		//Since the view matrix is yet to be transposed until it is rendered, I can use it for moving the camera.
		if (GetAsyncKeyState('W'))
		{
			XMMATRIX translate = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, -0.1f, 1
			};
			g_View = XMMatrixMultiply(g_View, translate);
		}

		if (GetAsyncKeyState('S'))
		{
			XMMATRIX translate = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0.1f, 1
			};
			g_View = XMMatrixMultiply(g_View, translate);
		}

		if (GetAsyncKeyState('A'))
		{
			XMMATRIX translate = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0.1f, 0, 0, 1
			};
			g_View = XMMatrixMultiply(g_View, translate);
		}

		if (GetAsyncKeyState('D'))
		{
			XMMATRIX translate = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			-0.1f, 0, 0, 1
			};
			g_View = XMMatrixMultiply(g_View, translate);
		}

		if (GetAsyncKeyState('Q'))
		{
			g_View *= XMMatrixRotationY(0.1f);
		}

		if (GetAsyncKeyState('E'))
		{
			g_View *= XMMatrixRotationY(-0.1f);
		}

		if (GetAsyncKeyState(32))
		{
			XMMATRIX translate = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, -0.1f, 0, 1
			};
			g_View = XMMatrixMultiply(g_View, translate);
		}

		if (GetAsyncKeyState('C'))
		{
			XMMATRIX translate = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0.1f, 0, 1
			};
			g_View = XMMatrixMultiply(g_View, translate);
		}
	}
};