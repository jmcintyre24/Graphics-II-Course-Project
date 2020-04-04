#pragma once
#include "defines.h"
#include "DDSTextureLoader.h"

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
	UINT clientWidth = 0, clientHeight = 0;
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

class Mesh : DrawClass
{
	struct ConstantBuffer
	{
		XMMATRIX mWorld;
		XMMATRIX mView;
		XMMATRIX mProjection;
		XMFLOAT4 vOutputColor;
	};

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>		renderTargetView = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>				depthStencil = nullptr;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>		depthStencilView = nullptr;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>			input = nullptr;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>			vertexshader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>			pixelshader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer>				vertexbuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer>				indexbuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer>				constantbuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	textureRV = nullptr;
	Microsoft::WRL::ComPtr<ID3D11SamplerState>			samplerLinear = nullptr;
	XMMATRIX											g_World;
	XMMATRIX											g_View;
	XMMATRIX											g_Projection;

public:
	struct SimpleVertex
	{
		XMFLOAT3 Pos;
		XMFLOAT3 Normal;
		XMFLOAT2 UV;
	};

	struct SimpleMesh
	{
		std::vector<SimpleVertex> vertexList;
		std::vector<unsigned int> indicesList;
	};

private: 	
	SimpleMesh* mesh = nullptr;

public:

	Mesh(GW::GRAPHICS::GDirectX11Surface _d3d11, GW::SYSTEM::GWindow _win, SimpleMesh* _mesh) : DrawClass(_d3d11, _win)
	{
		if (_mesh == nullptr)
		{
			std::cout << "Mesh was nullptr/Invalid\n";
		}

		mesh = _mesh;
		ID3D11Device* dev = nullptr;
		ID3D11DeviceContext* con = nullptr;
		+d3d11.GetDevice((void**)&dev);
		+d3d11.GetImmediateContext((void**)(&con));

		// Back Buffer setup
		{
			IDXGISwapChain* swp = nullptr;
			+d3d11.GetSwapchain((void**)(&swp));
			// Create a render target view
			ID3D11Texture2D* pBackBuffer = nullptr;
			if (FAILED(swp->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer))))
				return;

			if (FAILED(dev->CreateRenderTargetView(pBackBuffer, nullptr, renderTargetView.GetAddressOf())))
			{
				pBackBuffer->Release();
				return;
			}
			pBackBuffer->Release();
			swp->Release();

			// Create depth stencil texture
			D3D11_TEXTURE2D_DESC descDepth = {};
			descDepth.Width = clientWidth;
			descDepth.Height = clientHeight;
			descDepth.MipLevels = 1;
			descDepth.ArraySize = 1;
			descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			descDepth.SampleDesc.Count = 1;
			descDepth.SampleDesc.Quality = 0;
			descDepth.Usage = D3D11_USAGE_DEFAULT;
			descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			descDepth.CPUAccessFlags = 0;
			descDepth.MiscFlags = 0;
			if (FAILED(dev->CreateTexture2D(&descDepth, nullptr, depthStencil.GetAddressOf())))
				return;

			// Create the depth stencil view
			D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
			descDSV.Format = descDepth.Format;
			descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			descDSV.Texture2D.MipSlice = 0;

			if (FAILED(dev->CreateDepthStencilView(depthStencil.Get(), &descDSV, depthStencilView.GetAddressOf())))
				return;

			con->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());

			// Setup the viewport

		}

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
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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
		


		// Create Vertex Buffer
		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(SimpleVertex) * mesh->vertexList.size();
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA InitData = {};
		InitData.pSysMem = mesh->vertexList.data();
		if (FAILED(dev->CreateBuffer(&bd, &InitData, vertexbuffer.GetAddressOf())))
			return;

		// Set vertex buffer
		const UINT stride[] = { sizeof(SimpleVertex) };
		const UINT offset[] = { 0 };
		ID3D11Buffer* const buffs[] = { vertexbuffer.Get() };
		con->IASetVertexBuffers(0, ARRAYSIZE(buffs), buffs, stride, offset);

		// Create Index Buffer
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(int) * mesh->indicesList.size();
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		InitData.pSysMem = mesh->indicesList.data();
		if (FAILED(dev->CreateBuffer(&bd, &InitData, indexbuffer.GetAddressOf())))
			return;

		// Set Index Buffer
		con->IASetIndexBuffer(indexbuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// Create the constant buffer
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(ConstantBuffer);
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		if (FAILED(dev->CreateBuffer(&bd, nullptr, constantbuffer.GetAddressOf())))
			return;

		// LOADING TEXTURE //

		// Load the Texture
		
		if (FAILED(CreateDDSTextureFromFile(dev, L"Textures\\StoneHenge.dds", nullptr, textureRV.GetAddressOf())))
			return;

		// Create the sample state
		D3D11_SAMPLER_DESC sampDesc = {};
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		if (FAILED(dev->CreateSamplerState(&sampDesc, samplerLinear.GetAddressOf())))
			return;

		// ~~~~~~~~~~~ //

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
		if (mesh == nullptr)
			return;

		// Render a triangle
		ID3D11DeviceContext* con;
		ID3D11RenderTargetView* view;
		d3d11.GetImmediateContext((void**)&con);
		d3d11.GetRenderTargetView((void**)&view);

		// setup the pipeline
		//ID3D11RenderTargetView* const views[] = { view };
		//con->OMSetRenderTargets(ARRAYSIZE(views), views, nullptr);

		con->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);


		ConstantBuffer cb;
		cb.mWorld = XMMatrixTranspose(g_World);
		cb.mView = XMMatrixTranspose(g_View);
		cb.mProjection = XMMatrixTranspose(g_Projection);
		cb.vOutputColor = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
		con->UpdateSubresource(constantbuffer.Get(), 0, nullptr, &cb, 0, 0);

		// Render the mesh
		con->VSSetShader(vertexshader.Get(), nullptr, 0);
		con->VSSetConstantBuffers(0, 1, constantbuffer.GetAddressOf());
		con->PSSetShader(pixelshader.Get(), nullptr, 0);
		con->PSSetConstantBuffers(0, 1, constantbuffer.GetAddressOf());
		con->PSSetShaderResources(0, 1, textureRV.GetAddressOf());
		con->PSSetSamplers(0, 1, samplerLinear.GetAddressOf());
		con->IASetInputLayout(input.Get());
		con->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

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