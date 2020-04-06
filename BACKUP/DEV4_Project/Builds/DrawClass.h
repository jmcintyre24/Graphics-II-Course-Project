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

class Cube : DrawClass
{
	struct SimpleVertex
	{
		XMFLOAT3 Pos;
		XMFLOAT4 Color;
	};
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

public:
	Cube(GW::GRAPHICS::GDirectX11Surface _d3d11, GW::SYSTEM::GWindow _win) : DrawClass(_d3d11, _win)
	{
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
		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(SimpleVertex) * 8;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA InitData = {};
		InitData.pSysMem = vertices;
		if (FAILED(dev->CreateBuffer(&bd, &InitData, vertexbuffer.GetAddressOf())))
			return;

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
		// Create the constant buffer
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(ConstantBuffer);
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		if (FAILED(dev->CreateBuffer(&bd, nullptr, constantbuffer.GetAddressOf())))
			return;

		con->Release();
		dev->Release();
		return;
	}
	void Render(XMMATRIX g_World, XMMATRIX g_View, XMMATRIX g_Projection)
	{
		// Render a triangle
		ID3D11DeviceContext* con;
		ID3D11RenderTargetView* view;
		d3d11.GetImmediateContext((void**)&con);
		d3d11.GetRenderTargetView((void**)&view);

		//Set Index Buffer
		con->IASetIndexBuffer(indexbuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
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
		con->DrawIndexed(36, 0, 0);        // 36 vertices needed for 12 triangles in a triangle list
		con->Release();
		view->Release();
	}
};

class Mesh : DrawClass
{
	struct ConstantBuffer
	{
		XMMATRIX mWorld;
		XMMATRIX mView;
		XMMATRIX mProjection;
		XMFLOAT4 lightDir[2];
		XMFLOAT4 lightClr[2];
		XMFLOAT4 vOutputColor;
	};

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>		renderTargetView = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>				depthStencil = nullptr;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>		depthStencilView = nullptr;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>			input = nullptr;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>			vertexshader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>			pixelshader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>			pixelshaderLights = nullptr;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>			pixelshaderUnique = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer>				vertexbuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer>				indexbuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer>				constantbuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	textureRV = nullptr;
	Microsoft::WRL::ComPtr<ID3D11SamplerState>			samplerLinear = nullptr;
	XMMATRIX											g_World;
	XMMATRIX											g_View;
	XMMATRIX											g_Projection;

	bool doFlip = false;
	bool moveDirLight = false;
	bool ghostProtect = false;

	XMFLOAT4 lightDir[2], lightClr[2];

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

		// Compile the lighting pixel shader
		pPSBlob = nullptr;
		if (FAILED(DrawClass::CompileShaderFromFile(L"shaders.fx", "PSSolid", "ps_4_0", &pPSBlob)))
		{
			MessageBox(nullptr,
				L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
			return;
		}

		// Create the pixel shader
		if (FAILED(dev->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, pixelshaderLights.GetAddressOf())))
		{
			std::cout << (char*)errors->GetBufferPointer() << std::endl;
			pPSBlob->Release();
			return;
		}

		// Compile the lighting pixel shader
		pPSBlob = nullptr;
		if (FAILED(DrawClass::CompileShaderFromFile(L"shaders.fx", "PSUnique", "ps_4_0", &pPSBlob)))
		{
			MessageBox(nullptr,
				L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
			return;
		}

		// Create the pixel shader
		if (FAILED(dev->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, pixelshaderUnique.GetAddressOf())))
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

		// Set-up Lighting Variables
		{
			// Directional Lighting
			lightDir[0] = { -0.577f, 0.577f, -0.577f, 1.0f };
			lightClr[0] = { 0.85f, 0.85f, 0.85f, 1.0f };
			// Positional Lighting
			lightDir[1] = { 0.0f, 0.2f, -1.0f, 1.0f };
			lightClr[1] = { 0.7f, 0.2f, 0.2f, 1.0f };
		}

		con->Release();
		dev->Release();
		return;
	}

	void Render()
	{
		if (mesh == nullptr)
			return;

		// Update our time
		static float t = 0.0f;

		static ULONGLONG timeStart = 0;
		ULONGLONG timeCur = GetTickCount64();
		if (timeStart == 0)
			timeStart = timeCur;
		t = (timeCur - timeStart) / 1500.0f;

		// Grab the context and view.
		ID3D11DeviceContext* con;
		ID3D11RenderTargetView* view;
		d3d11.GetImmediateContext((void**)&con);
		d3d11.GetRenderTargetView((void**)&view);
		
		// Update poslight for attenuation
		if (!doFlip)
		{
			lightClr[1].x -= t;
			lightClr[1].y -= t;
			lightClr[1].z -= t;

			if(!XMVector3Greater({ lightClr[1].x, lightClr[1].y, lightClr[1].z }, { 0.01f,0.01f,0.01f }))
				doFlip = true;
		}
		else if (doFlip)
		{
			lightClr[1].x += t;
			lightClr[1].y += t;
			lightClr[1].z += t;

			if (XMVector3Greater({ lightClr[1].x, lightClr[1].y, lightClr[1].z }, { 0.8f, 0.8f, 0.8f }))
				doFlip = false;
		}

		// Rotate the directional light
		if (!moveDirLight)
		{
			XMMATRIX mRotate = XMMatrixRotationY(0.5f * t);
			XMVECTOR vLightDir = XMLoadFloat4(&lightDir[0]);
			vLightDir = XMVector3Transform(vLightDir, mRotate);
			XMStoreFloat4(&lightDir[0], vLightDir);
		}

		// Rotate the second light around the origin
		XMMATRIX mRotate = XMMatrixRotationY(-2.0f * t);
		XMVECTOR vLightDir = XMLoadFloat4(&lightDir[1]);
		vLightDir = XMVector3Transform(vLightDir, mRotate);
		XMStoreFloat4(&lightDir[1], vLightDir);

		con->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

		// Constant Buffer to communicate with the shader's values on the GPU
		ConstantBuffer cb;
		cb.mWorld = XMMatrixTranspose(g_World);
		cb.mView = XMMatrixTranspose(g_View);
		cb.mProjection = XMMatrixTranspose(g_Projection);
		// Directional Light [0]
		cb.lightDir[0] = lightDir[0];
		cb.lightClr[0] = lightClr[0];
		// Point Light [1]
		cb.lightDir[1] = lightDir[1];
		cb.lightClr[1] = lightClr[1];
		cb.vOutputColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
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

		// Render the lighting sources as the model (For now)
		for (int i = 0; i < 2; i++)
		{
			XMMATRIX mLight = XMMatrixTranslationFromVector(5.0f * XMLoadFloat4(&lightDir[i]));
			XMMATRIX mLightScale = XMMatrixScaling(0.2f, 0.2f, 0.2f);
			mLight = mLightScale * mLight;

			// Update the world variable to reflect the current light
			cb.mWorld = XMMatrixTranspose(mLight);
			cb.vOutputColor = lightClr[i];
			con->UpdateSubresource(constantbuffer.Get(), 0, nullptr, &cb, 0, 0);

			if(i == 0)
				con->PSSetShader(pixelshaderLights.Get(), nullptr, 0);
			else
				con->PSSetShader(pixelshaderUnique.Get(), nullptr, 0);
			con->DrawIndexed(mesh->indicesList.size(), 0, 0);
		}

		timeStart = timeCur;

		con->Release();
		view->Release();
	}

	void UserInput()
	{
		// Rotate the objects.
		if (GetAsyncKeyState('J'))
		{
			g_World *= XMMatrixRotationY(0.1f);
		}

		if (GetAsyncKeyState('L'))
		{
			g_World *= XMMatrixRotationY(-0.1f);
		}

		// Toggle the ability to move directional light with M1
		if ((GetKeyState('Z') & 0x8000) && ghostProtect == false )
		{
			ghostProtect = true;
			moveDirLight = !moveDirLight;
		}
		else if ((GetKeyState('Z') == 0x0000) && ghostProtect == true)
		{
			moveDirLight = !moveDirLight;
			ghostProtect = false;
		}

		// Look around movement
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
				XMMATRIX oldView = g_View;

				g_View = XMMatrixTranslation(0.0f, 0.0f, 0.0f);

				g_View *= XMMatrixRotationX(-0.05f);

				g_View = XMMatrixMultiply(oldView, g_View);

			}
			else if (diffY > mouseThreshold)
			{
				XMMATRIX oldView = g_View;

				g_View = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
				
				g_View *= XMMatrixRotationX(0.05f);

				g_View = XMMatrixMultiply(oldView, g_View);
				//g_View *= XMMatrixRotationX(0.05f);
			}

			//Set it back to the center
			SetCursorPos(cosX, cosY);
		}

		// Move Directional Light
		if (moveDirLight && (GetKeyState(VK_LBUTTON) & 0x100) != 0)
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

			XMMATRIX mRotate = XMMatrixRotationY(0.0125f * (diffX + diffY) );
			XMVECTOR vLightDir = XMLoadFloat4(&lightDir[0]);
			vLightDir = XMVector3Transform(vLightDir, mRotate);
			XMStoreFloat4(&lightDir[0], vLightDir);

			//Set it back to the center
			SetCursorPos(cosX, cosY);
		}

		// Since the view matrix is yet to be transposed until it is rendered, I can use it for moving the camera.
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