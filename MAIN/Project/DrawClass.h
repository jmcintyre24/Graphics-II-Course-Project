#pragma once
#include <zmouse.h>
#include "defines.h"
#include "DDSTextureLoader.h"

// Base class for drawing objects
class DrawClass
{
public:
	DrawClass(GW::GRAPHICS::GDirectX11Surface _d3d11, GW::SYSTEM::GWindow _win)
	{
		win = _win;
		d3d11 = _d3d11;

		+win.GetWidth(width);
		+win.GetHeight(height);

		+win.GetClientWidth(clientWidth);
		+win.GetClientHeight(clientHeight);
	}

	// Used for compiling shaders
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
				OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer())); // Print to output window.
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
};

class Mesh : DrawClass
{
public:
	struct SimpleVertex
	{
		XMFLOAT4 Pos;
		XMFLOAT3 Normal;
		XMFLOAT2 UV;
	};

	struct SimpleMesh
	{
		std::vector<SimpleVertex> vertexList;
		std::vector<unsigned int> indicesList;
	};

private:
	struct ConstantBuffer
	{
		XMMATRIX mWorld;
		XMMATRIX mView;
		XMMATRIX mProjection;
		XMFLOAT4 lightDir[3];
		XMFLOAT4 lightClr[3];
		XMFLOAT4 vOutputColor;
		float time;
		float cone;
	};

	struct UniqueBuffer
	{
		XMFLOAT4 timePos;
	};

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>		renderTargetView = nullptr;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>			input = nullptr;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>			vertexshader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>			vertexshaderwave = nullptr;
	Microsoft::WRL::ComPtr<ID3D11GeometryShader>		geoshader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11GeometryShader>		geoshaderwave = nullptr;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>			pixelshader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>			pixelshaderSolid = nullptr;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>			pixelshaderNoLights = nullptr;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>			pixelshaderUnique = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer>				vertexbuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer>				indexbuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer>				constantbuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	textureRV = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	normRV = nullptr;
	Microsoft::WRL::ComPtr<ID3D11SamplerState>			samplerLinear = nullptr;
	XMMATRIX											g_World;
	XMMATRIX											g_View;
	XMMATRIX											g_Projection;
	XMMATRIX											stor_World;
	XMMATRIX											stor_View;
	XMMATRIX											stor_Projection;

	bool doFlip = false;
	bool moveDirLight = false;
	bool ghostProtect = false, ghostProtectZ = false;

	float zoom = 0;
	float nearP = 0.01f, farP = 100.0f;

	XMFLOAT4 lightDir[3], lightClr[3]; // Should've used a structure here - Note for 'next' time.
	float cone = 20.0f;
	SimpleMesh* mesh = nullptr;


	// For Cube - Will try to move to seperate class once working.
	Microsoft::WRL::ComPtr<ID3D11Buffer>				c_vertexbuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer>				c_indexbuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer>				u_constantbuffer = nullptr;

	// Generate a hard-coded cube. (Will hopefully move this out)
	void CreateCube(ID3D11Device* dev, ID3D11DeviceContext* con)
	{
		// Create vertex buffer
		SimpleVertex vertices[] =
		{
			{ XMFLOAT4(-1.0f, 1.0f, -1.0f, 1.0f),	XMFLOAT3(0.0f, 1.0f, 0.0f),		XMFLOAT2(1.0f, 0.0f)},
			{ XMFLOAT4(1.0f, 1.0f, -1.0f, 1.0f),	XMFLOAT3(0.0f, 1.0f, 0.0f),		XMFLOAT2(0.0f, 0.0f)},
			{ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),		XMFLOAT3(0.0f, 1.0f, 0.0f),		XMFLOAT2(0.0f, 1.0f)},
			{ XMFLOAT4(-1.0f, 1.0f, 1.0f, 1.0f),	XMFLOAT3(0.0f, 1.0f, 0.0f),		XMFLOAT2(1.0f, 1.0f)},

			{ XMFLOAT4(-1.0f, -1.0f, -1.0f, 1.0f),	XMFLOAT3(0.0f, -1.0f, 0.0f),	XMFLOAT2(0.0f, 0.0f)},
			{ XMFLOAT4(1.0f, -1.0f, -1.0f, 1.0f),	XMFLOAT3(0.0f, -1.0f, 0.0f),	XMFLOAT2(1.0f, 0.0f)},
			{ XMFLOAT4(1.0f, -1.0f, 1.0f, 1.0f),	XMFLOAT3(0.0f, -1.0f, 0.0f),	XMFLOAT2(1.0f, 1.0f)},
			{ XMFLOAT4(-1.0f, -1.0f, 1.0f, 1.0f),	XMFLOAT3(0.0f, -1.0f, 0.0f),	XMFLOAT2(0.0f, 1.0f)},

			{ XMFLOAT4(-1.0f, -1.0f, 1.0f, 1.0f),	XMFLOAT3(-1.0f, 0.0f, 0.0f),	XMFLOAT2(0.0f, 1.0f)},
			{ XMFLOAT4(-1.0f, -1.0f, -1.0f, 1.0f),	XMFLOAT3(-1.0f, 0.0f, 0.0f),	XMFLOAT2(1.0f, 1.0f)},
			{ XMFLOAT4(-1.0f, 1.0f, -1.0f, 1.0f),	XMFLOAT3(-1.0f, 0.0f, 0.0f),	XMFLOAT2(1.0f, 0.0f)},
			{ XMFLOAT4(-1.0f, 1.0f, 1.0f, 1.0f),	XMFLOAT3(-1.0f, 0.0f, 0.0f),	XMFLOAT2(0.0f, 0.0f)},

			{ XMFLOAT4(1.0f, -1.0f, 1.0f, 1.0f),	XMFLOAT3(1.0f, 0.0f, 0.0f),		XMFLOAT2(1.0f, 1.0f)},
			{ XMFLOAT4(1.0f, -1.0f, -1.0f, 1.0f),	XMFLOAT3(1.0f, 0.0f, 0.0f),		XMFLOAT2(0.0f, 1.0f)},
			{ XMFLOAT4(1.0f, 1.0f, -1.0f, 1.0f),	XMFLOAT3(1.0f, 0.0f, 0.0f),		XMFLOAT2(0.0f, 0.0f)},
			{ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),		XMFLOAT3(1.0f, 0.0f, 0.0f),		XMFLOAT2(1.0f, 0.0f)},

			{ XMFLOAT4(-1.0f, -1.0f, -1.0f, 1.0f),	XMFLOAT3(0.0f, 0.0f, -1.0f),	XMFLOAT2(0.0f, 1.0f)},
			{ XMFLOAT4(1.0f, -1.0f, -1.0f, 1.0f),	XMFLOAT3(0.0f, 0.0f, -1.0f),	XMFLOAT2(1.0f, 1.0f)},
			{ XMFLOAT4(1.0f, 1.0f, -1.0f, 1.0f),	XMFLOAT3(0.0f, 0.0f, -1.0f),	XMFLOAT2(1.0f, 0.0f)},
			{ XMFLOAT4(-1.0f, 1.0f, -1.0f, 1.0f),	XMFLOAT3(0.0f, 0.0f, -1.0f),	XMFLOAT2(0.0f, 0.0f)},

			{ XMFLOAT4(-1.0f, -1.0f, 1.0f, 1.0f),	XMFLOAT3(0.0f, 0.0f, 1.0f),		XMFLOAT2(1.0f, 1.0f)},
			{ XMFLOAT4(1.0f, -1.0f, 1.0f, 1.0f),	XMFLOAT3(0.0f, 0.0f, 1.0f),		XMFLOAT2(0.0f, 1.0f)},
			{ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),		XMFLOAT3(0.0f, 0.0f, 1.0f),		XMFLOAT2(0.0f, 0.0f)},
			{ XMFLOAT4(-1.0f, 1.0f, 1.0f, 1.0f),	XMFLOAT3(0.0f, 0.0f, 1.0f),		XMFLOAT2(1.0f, 0.0f)},
		};
		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(SimpleVertex) * 24;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA InitData = {};
		InitData.pSysMem = vertices;
		if (FAILED(dev->CreateBuffer(&bd, &InitData, c_vertexbuffer.GetAddressOf())))
		{
			DebugBreak();
			return;
		}
		// Create index buffer
		unsigned int indices[] =
		{
			3,1,0,
			2,1,3,

			6,4,5,
			7,4,6,

			11,9,8,
			10,9,11,

			14,12,13,
			15,12,14,

			19,17,16,
			18,17,19,

			22,20,21,
			23,20,22
		};
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(unsigned int) * 36;        // 36 vertices needed for 12 triangles in a triangle list
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		InitData.pSysMem = indices;
		if (FAILED(dev->CreateBuffer(&bd, &InitData, c_indexbuffer.GetAddressOf())))
		{
			DebugBreak();
			return;
		}
		// Create the unique constant buffer
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(UniqueBuffer);
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		if (FAILED(dev->CreateBuffer(&bd, nullptr, u_constantbuffer.GetAddressOf())))
		{
			DebugBreak();
			return;
		}
	}

	// For Grid - Same thing as the cube, my end goal would be to move everything out and create a much better pipeline for rendering.
	Microsoft::WRL::ComPtr<ID3D11Buffer>				g_vertexbuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer>				g_indexbuffer = nullptr;

	// Create index buffer
	std::vector<unsigned int> gridIndices;

	// Generate a proceduraly 3D grid.
	void CreateGrid(ID3D11Device* dev, ID3D11DeviceContext* con)
	{
		// Create vertex buffer
		std::vector<SimpleVertex> verts;

		for (int z = -50; z < 50; z++)
		{
			for (int x = -50; x < 50; x++)
			{
				verts.push_back({ XMFLOAT4((x / 50.0f) * 5.0f, 0.0f, (z / 50.0f) * 5.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), });
			}
		}

		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(SimpleVertex) * verts.size();
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA InitData = {};
		InitData.pSysMem = verts.data();
		if (FAILED(dev->CreateBuffer(&bd, &InitData, g_vertexbuffer.GetAddressOf())))
		{
			DebugBreak();
			return;
		}

		// Create Vertical Lines
		for (int i = 0; i < (verts.size() - 100); i++)
		{
			gridIndices.push_back(i);
			gridIndices.push_back(i + 100);
		}
		// Create Horizontal Lines
		for (int x = 0; x < verts.size(); x += 100)
		{
			for (int i = 0; i < 99; i++)
			{
				gridIndices.push_back(i + x);
				gridIndices.push_back(i + 1 + x);
			}
		}

		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(unsigned int) * gridIndices.size();        // 36 vertices needed for 12 triangles in a triangle list
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		InitData.pSysMem = gridIndices.data();
		if (FAILED(dev->CreateBuffer(&bd, &InitData, g_indexbuffer.GetAddressOf())))
		{
			DebugBreak();
			return;
		}
	}

	// Render the grid
	void RenderGrid(ID3D11DeviceContext* con, ID3D11RenderTargetView* view, ConstantBuffer& cb)
	{
		// Change Topology to Lines
		con->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

		// Set vertex buffer
		const UINT c_stride[] = { sizeof(SimpleVertex) };
		const UINT c_offset[] = { 0 };
		ID3D11Buffer* const c_buffs[] = { g_vertexbuffer.Get() };
		con->IASetVertexBuffers(0, ARRAYSIZE(c_buffs), c_buffs, c_stride, c_offset);

		// Set Index Buffer
		con->IASetIndexBuffer(g_indexbuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// Update the world variable to reflect the current light
		XMFLOAT4 pos = { 0.0f, -0.5f, 0.0f, 0.0f };
		XMMATRIX w_Grid = XMMatrixTranslationFromVector(5.0f * XMLoadFloat4(&pos));
		cb.mWorld = XMMatrixTranspose(w_Grid);
		cb.vOutputColor = {0.1f, 0.2f, 1.0f, 1.0f};
		con->UpdateSubresource(constantbuffer.Get(), 0, nullptr, &cb, 0, 0);

		// Update VS, GS, and PS's constant buffer to unique
		con->VSSetShader(vertexshaderwave.Get(), nullptr, 0);
		con->VSSetConstantBuffers(0, 1, constantbuffer.GetAddressOf());
		//con->GSSetShader(geoshaderwave.Get(), 0, 0);
		//con->GSSetConstantBuffers(0, 1, constantbuffer.GetAddressOf());
		con->PSSetConstantBuffers(0, 1, constantbuffer.GetAddressOf());
		con->PSSetShader(pixelshaderSolid.Get(), nullptr, 0);
	
		con->DrawIndexed(gridIndices.size(), 0, 0);
		
		// Reset Geometry Shader so it doesn't affect everything else.
		//con->GSSetShader(nullptr, 0, 0);
		con->VSSetShader(vertexshader.Get(), nullptr, 0);

		// Change Topology to Triangles
		con->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	// For Skybox Generation
	Microsoft::WRL::ComPtr<ID3D11VertexShader>			SKBvertexshader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>			SKBpixelshader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer>				SKBvertex_Buffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>			SKBinput = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	SKBtextureRV = nullptr;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState>		depthStencilState = nullptr;

	// Reflection Cube Variables
	//XMFLOAT4											refCube = {0.1f, 0.0f, 0.2f, 1.0f};
	//XMFLOAT4											clrCube = {0.4f, 0.4f, 1.0f, 1.0f };

	// Render out the cube that reflects the skybox.
	void renderReflectionCube(ID3D11DeviceContext* con, ID3D11RenderTargetView* view, ConstantBuffer& cb)
	{
		// Render the light sources as cubes (So they are visible)
		// Set vertex buffer
		const UINT c_stride[] = { sizeof(SimpleVertex) };
		const UINT c_offset[] = { 0 };
		ID3D11Buffer* const c_buffs[] = { c_vertexbuffer.Get() };
		con->IASetVertexBuffers(0, ARRAYSIZE(c_buffs), c_buffs, c_stride, c_offset);

		// Set Index Buffer
		con->IASetIndexBuffer(c_indexbuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// Start rendering the cube.
		XMMATRIX mLight = XMMatrixTranslationFromVector(1.0f * XMLoadFloat4(&lightDir[1]));
		XMMATRIX mLightScale = XMMatrixScaling(0.5f, 0.5f, 0.5f);
		mLight = mLightScale * mLight;

		// Update the world variable to reflect the current light
		cb.mWorld = XMMatrixTranspose(mLight);
		cb.vOutputColor = lightClr[1];
		con->UpdateSubresource(constantbuffer.Get(), 0, nullptr, &cb, 0, 0);

		// Be sure the constant buffer is still the contsant buffer.
		con->PSSetConstantBuffers(0, 1, constantbuffer.GetAddressOf());
		con->PSSetShader(pixelshaderSolid.Get(), nullptr, 0);

		// Draw it out
		con->DrawIndexed(36, 0, 0);
	}

	// For Render to Texture
	Microsoft::WRL::ComPtr<ID3D11Texture2D>				RTrenderTargetTexture = nullptr;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>		RTrenderTargetView = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	RTshaderResourceView = nullptr;

	XMMATRIX											vp_two_World;
	XMMATRIX											vp_two_View;
	XMMATRIX											vp_two_Projection;

	XMMATRIX											rtt_View;
	XMMATRIX											rtt_Projection;
	float clr[4] = { 0.2f, 0.2f, 0.5f, 1 };

	// Render to Texture Initialization
	void InitRTT(ID3D11Device* dev, ID3D11DeviceContext* con)
	{
		D3D11_TEXTURE2D_DESC textureDesc;
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

		// Render to Texture
		// Initialize the texture description.
		ZeroMemory(&textureDesc, sizeof(textureDesc));

		// Setup the texture description.
		textureDesc.Width = clientWidth;
		textureDesc.Height = clientHeight;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;

		// Create the texture
		if(FAILED(dev->CreateTexture2D(&textureDesc, NULL, RTrenderTargetTexture.GetAddressOf())))
		{
			DebugBreak();
			return;
		}

		// Setup the render target view.
		renderTargetViewDesc.Format = textureDesc.Format;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D.MipSlice = 0;

		// Create the render target view.
		if(FAILED(dev->CreateRenderTargetView(RTrenderTargetTexture.Get(), &renderTargetViewDesc, RTrenderTargetView.GetAddressOf())))
		{
			DebugBreak();
			return;
		}

		// Setup the shader resource view.
		shaderResourceViewDesc.Format = textureDesc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;

		// Create the shader resource view.
		if(FAILED(dev->CreateShaderResourceView(RTrenderTargetTexture.Get(), &shaderResourceViewDesc, RTshaderResourceView.GetAddressOf())))
		{
			DebugBreak();
			return;
		}

		// Initialize the view matrix
		XMVECTOR Eye = XMVectorSet(0.0f, 1.0f + 15.0f, -5.0f, 100.0f);
		XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		rtt_View = XMMatrixLookAtLH(Eye, At, Up);

		// Orthographic Projection Matrix
		rtt_Projection = XMMatrixOrthographicLH(8, 8, nearP, farP);
		// Isometric Projection Matrix
		//rtt_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, DrawClass::width / (FLOAT)DrawClass::height, nearP, farP);
	}

	// Draw the object in the seperate 'scene'
	void DrawBehind(ID3D11DeviceContext* con, ID3D11RenderTargetView* view, ConstantBuffer& cb, UINT size)
	{
		ID3D11DepthStencilView* depthview = nullptr;
		+d3d11.GetDepthStencilView((void**)&depthview);

		con->ClearRenderTargetView(RTrenderTargetView.Get(), clr);

		con->OMSetRenderTargets(1, RTrenderTargetView.GetAddressOf(), depthview);

		//cb.mWorld = XMMatrixTranspose(rtt_World);
		cb.mView = XMMatrixTranspose(rtt_View);
		cb.mProjection = XMMatrixTranspose(rtt_Projection);
		cb.vOutputColor = { 1.0f, 0.5f, 1.0f, 1.0f };
		con->UpdateSubresource(constantbuffer.Get(), 0, nullptr, &cb, 0, 0);

		con->DrawIndexed(size, 0, 0);

		con->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthview);
		con->ClearDepthStencilView(depthview, D3D11_CLEAR_DEPTH, 1.0f, 0);
		depthview->Release();

		XMVECTOR det;
		cb.mWorld = g_World;
		cb.mView = XMMatrixTranspose(XMMatrixInverse(&det, g_View));
		cb.mProjection = XMMatrixTranspose(g_Projection);
		con->UpdateSubresource(constantbuffer.Get(), 0, nullptr, &cb, 0, 0);
	}

	XMFLOAT4											posRTTCube = {0.0f, 2.5f, 0.0f, 1.0f};
	XMFLOAT4											clrRTTCube = {1.0f, 1.0f, 1.0f, 1.0f };

	void RenderRTT(ID3D11DeviceContext* con, ID3D11RenderTargetView* view, ConstantBuffer& cb, UINT size)
	{
		// Set vertex buffer
		const UINT c_stride[] = { sizeof(SimpleVertex) };
		const UINT c_offset[] = { 0 };
		ID3D11Buffer* const c_buffs[] = { c_vertexbuffer.Get() };
		con->IASetVertexBuffers(0, ARRAYSIZE(c_buffs), c_buffs, c_stride, c_offset);

		// Set Index Buffer
		con->IASetIndexBuffer(c_indexbuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// Start rendering the cube.
		XMMATRIX mLight = XMMatrixTranslationFromVector(1.0f * XMLoadFloat4(&posRTTCube));
		XMMATRIX mLightScale = XMMatrixScaling(.5f, .5f, .5f);
		mLight = mLightScale * mLight;

		// Update the world variable to reflect the current light
		cb.mWorld = XMMatrixTranspose(mLight);
		//cb.mView = rtt_View;
		//cb.mProjection = rtt_Projection;
		cb.vOutputColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		con->UpdateSubresource(constantbuffer.Get(), 0, nullptr, &cb, 0, 0);

		// Be sure the constant buffer is still the contsant buffer.
		con->PSSetShader(pixelshaderNoLights.Get(), nullptr, 0);
		con->PSSetConstantBuffers(0, 1, constantbuffer.GetAddressOf());
		con->PSSetShaderResources(0, 1, RTshaderResourceView.GetAddressOf());
		con->PSSetSamplers(0, 1, samplerLinear.GetAddressOf());

		// Draw it out
		con->DrawIndexed(size, 0, 0);
	}

	D3D11_VIEWPORT										vp_one = { 0, 0, (float) clientWidth, (float) clientHeight, 0, 1, };

	D3D11_VIEWPORT										vp_two = { (float) clientWidth / 1.5f, 0, (float) clientWidth / 3.0f, (float) clientHeight / 3.0f, 0, 1, };

public:

	Mesh(GW::GRAPHICS::GDirectX11Surface _d3d11, GW::SYSTEM::GWindow _win, SimpleMesh* _mesh, const wchar_t* texturePath, const wchar_t* normPath) : DrawClass(_d3d11, _win)
	{
		if (_mesh == nullptr)
		{
			std::cout << "Mesh was nullptr/Invalid\n";
		}

		mesh = _mesh;
		ID3D11Device* dev = nullptr;
		ID3D11DeviceContext* con = nullptr;
		ID3D11DepthStencilView* depthview = nullptr;
		+d3d11.GetDevice((void**)&dev);
		+d3d11.GetImmediateContext((void**)(&con));
		+d3d11.GetDepthStencilView((void**)&depthview);

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

			con->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthview);
			depthview->Release();
		}

		// Creation of DEPTH stencil desc
		D3D11_DEPTH_STENCIL_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_DEPTH_STENCIL_DESC));
		desc.DepthEnable = true;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

		if (FAILED(dev->CreateDepthStencilState(&desc, &depthStencilState)))
		{
			DebugBreak();
			return;
		}

		// Compile the vertex shader
		ID3DBlob* pVSBlob = nullptr;
		if (FAILED(DrawClass::CompileShaderFromFile(L"Shaders\\shaders.fx", "VS", "vs_4_0", &pVSBlob)))
		{
			DebugBreak();
			return;
		}

		// Create the vertex shader
		if (FAILED(dev->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, vertexshader.GetAddressOf())))
		{
			DebugBreak();
			pVSBlob->Release();
			return;
		}

		// Compile the vertex shader for the wave.
		pVSBlob = nullptr;
		if (FAILED(DrawClass::CompileShaderFromFile(L"Shaders\\shaders.fx", "VSWave", "vs_4_0", &pVSBlob)))
		{
			DebugBreak();
			return;
		}

		// Create the vertex shader for the wave.
		if (FAILED(dev->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, vertexshaderwave.GetAddressOf())))
		{
			DebugBreak();
			pVSBlob->Release();
			return;
		}

		// Define the input layout
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		UINT numElements = ARRAYSIZE(layout);

		// Create the input layout
		if (FAILED(dev->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), input.GetAddressOf())))
		{
			DebugBreak();
			pVSBlob->Release();
			return;
		}

		con->IASetInputLayout(input.Get());

		// Skybox VS
		// Compile the Skybox vertex shader
		pVSBlob = nullptr;
		if (FAILED(DrawClass::CompileShaderFromFile(L"Shaders\\shaders.fx", "SKYBOX_VS", "vs_4_0", &pVSBlob)))
		{
			DebugBreak();
			return;
		}

		// Create the Skybox vertex shader
		if (FAILED(dev->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, SKBvertexshader.GetAddressOf())))
		{
			DebugBreak();
			pVSBlob->Release();
			return;
		}

		// Define the input layout
		D3D11_INPUT_ELEMENT_DESC SKBlayout[] =
		{
			{ "SV_POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 2, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		numElements = ARRAYSIZE(SKBlayout);

		// Create the input layout
		if (FAILED(dev->CreateInputLayout(SKBlayout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), SKBinput.GetAddressOf())))
		{
			DebugBreak();
			pVSBlob->Release();
			return;
		}

		pVSBlob->Release();

		// Create the Base Geometry Shader
		ID3DBlob* pGSBlob = nullptr;

		if (FAILED(DrawClass::CompileShaderFromFile(L"Shaders\\shaders.fx", "GS", "gs_4_0", &pGSBlob)))
		{
			DebugBreak();
			pGSBlob->Release();
			return;
		}

		if (FAILED(dev->CreateGeometryShader(pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), NULL, &geoshader)))
		{
			DebugBreak();
			pGSBlob->Release();
			return;
		}

		// Create the Wave Geometry Shader
		pGSBlob = nullptr;
		if (FAILED(DrawClass::CompileShaderFromFile(L"Shaders\\shaders.fx", "GSWave", "gs_4_0", &pGSBlob)))
		{
			DebugBreak();
			pGSBlob->Release();
			return;
		}

		if (FAILED(dev->CreateGeometryShader(pGSBlob->GetBufferPointer(), pGSBlob->GetBufferSize(), NULL, &geoshaderwave)))
		{
			DebugBreak();
			pGSBlob->Release();
			return;
		}

		pGSBlob->Release();

		// Compile the pixel shaders
		ID3DBlob* pPSBlob = nullptr;
		if (FAILED(DrawClass::CompileShaderFromFile(L"Shaders\\shaders.fx", "PS", "ps_4_0", &pPSBlob)))
		{
			MessageBox(nullptr,
				L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
			return;
		}

		// Create the pixel shader
		if (FAILED(dev->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, pixelshader.GetAddressOf())))
		{
			pPSBlob->Release();
			return;
		}

		// Compile the lighting pixel shader
		pPSBlob = nullptr;
		if (FAILED(DrawClass::CompileShaderFromFile(L"Shaders\\shaders.fx", "PSSolid", "ps_4_0", &pPSBlob)))
		{
			MessageBox(nullptr,
				L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
			return;
		}

		// Create the pixel shader
		if (FAILED(dev->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, pixelshaderSolid.GetAddressOf())))
		{
			pPSBlob->Release();
			return;
		}

		// Compile the unique lighting pixel shader
		pPSBlob = nullptr;
		if (FAILED(DrawClass::CompileShaderFromFile(L"Shaders\\shaders.fx", "PSNoLights", "ps_4_0", &pPSBlob)))
		{
			MessageBox(nullptr,
				L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
			DebugBreak();
			return;
		}

		// Create the unique pixel shader
		if (FAILED(dev->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, pixelshaderNoLights.GetAddressOf())))
		{
			pPSBlob->Release();
			DebugBreak();
			return;
		}
		pPSBlob->Release();

		// Compile the unique lighting pixel shader
		pPSBlob = nullptr;
		if (FAILED(DrawClass::CompileShaderFromFile(L"Shaders\\shaders.fx", "PSUnique", "ps_4_0", &pPSBlob)))
		{
			MessageBox(nullptr,
				L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
			DebugBreak();
			return;
		}

		// Create the unique pixel shader
		if (FAILED(dev->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, pixelshaderUnique.GetAddressOf())))
		{
			pPSBlob->Release();
			DebugBreak();
			return;
		}
		pPSBlob->Release();
		
		//Compile skybox pixel shader
		pPSBlob = nullptr;
		if (FAILED(DrawClass::CompileShaderFromFile(L"Shaders\\shaders.fx", "SKYBOX_PS", "ps_4_0", &pPSBlob)))
		{
			MessageBox(nullptr,
				L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
			DebugBreak();
			return;
		}

		// Create skybox pixel shader
		if (FAILED(dev->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, SKBpixelshader.GetAddressOf())))
		{
			pPSBlob->Release();
			DebugBreak();
			return;
		}
		pPSBlob->Release();

		// Create a cube to store and render later.
		CreateCube(dev, con);

		// Create the grid
		CreateGrid(dev, con);

		// Create Vertex Buffer
		D3D11_BUFFER_DESC bd = {};
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(SimpleVertex) * mesh->vertexList.size();
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA InitData = {};
		InitData.pSysMem = mesh->vertexList.data();
		if (FAILED(dev->CreateBuffer(&bd, &InitData, vertexbuffer.GetAddressOf())))
		{
			DebugBreak();
			return;
		}
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
		{
			DebugBreak();
			return;
		}

		// Set Index Buffer
		con->IASetIndexBuffer(indexbuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// Create the constant buffer
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(ConstantBuffer);
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		if (FAILED(dev->CreateBuffer(&bd, nullptr, constantbuffer.GetAddressOf())))
		{
			DebugBreak();
			return;
		}

		// LOADING TEXTURE //

		// Load the Texture
		
		if (FAILED(CreateDDSTextureFromFile(dev, texturePath, nullptr, textureRV.GetAddressOf())))
		{
			DebugBreak();
			return;
		}

		// Load Normal Map
		if (FAILED(CreateDDSTextureFromFile(dev, normPath, nullptr, normRV.GetAddressOf())))
		{
			DebugBreak();
			return;
		}

		// SKYBOX Texture
		if (FAILED(CreateDDSTextureFromFile(dev, L"Textures\\SunsetSkybox.dds", nullptr, SKBtextureRV.GetAddressOf())))
		{
			DebugBreak();
			return;
		}

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

		XMVECTOR det;
		g_View = XMMatrixInverse(&det, g_View);

		// Initialize the projection matrix
		g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, DrawClass::width / (FLOAT)DrawClass::height, nearP, farP);

		// Set-up Lighting Variables
		{
			// Directional Lighting
			lightDir[0] = { -0.577f, 0.577f, -0.577f, 1.0f };
			lightClr[0] = { 0.6f, 0.6f, 0.6f, 1.0f };
			// Positional Lighting
			lightDir[1] = { 0.0f, 0.2f, -1.0f, 1.0f };
			lightClr[1] = { 0.0f, 0.8f, 0.8f, 1.0f };
			// Spot Light
			lightDir[2] = { 0.0f, 0.577f, -0.577f, 1.0f };
			lightClr[2] = { 1.0f, 0.0f, 0.0f, 1.0f };
		}

		InitRTT(dev, con);

		// Setup viewport two's perspective and view matrices.
		Eye = XMVectorSet(0.0f, 3.0f, -1.0f, 1.0f);
		At = XMVectorSet(0.0f, -2.0f, 0.0f, 0.0f);
		Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		vp_two_View = XMMatrixLookAtLH(Eye, At, Up);

		vp_two_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, DrawClass::width / (FLOAT)DrawClass::height, nearP, farP);

		con->Release();
		dev->Release();
		return;
	}

	void Render(UINT flag = 1)
	{
		if (mesh == nullptr)
			return;

		// Update time
		static float t = 0.0f, tUpToOne = 0.0f, tTotal;

		static ULONGLONG timePerFrame = 0, timeStart = 0;

		ULONGLONG timeCur = GetTickCount64();
		if (timePerFrame == 0)
			timePerFrame = timeCur;
		t = (timeCur - timePerFrame) / 1500.0f;
		if (timeStart == 0)
			timeStart = timeCur;
		tUpToOne = (timeCur - timeStart) / 1000.0f;
		tTotal += t * 2.0f;

		// Reset the total time with that of the sine wave. (2 * pi)
		if (tTotal > 6.28f)
			tTotal = 0;

		// To cause a pulse for the Unique Pixel Shader
		if (tUpToOne > 1)
		{
			timeStart = timeCur;
		}

		// Grab the context and view.
		ID3D11DeviceContext* con;
		ID3D11RenderTargetView* view;
		d3d11.GetImmediateContext((void**)&con);
		d3d11.GetRenderTargetView((void**)&view);
		
		// Set the viewport.
		//con->ClearRenderTargetView(renderTargetView.Get(), clr);
		if (flag == 1)
			con->RSSetViewports(1, &vp_one);
		else
			con->RSSetViewports(1, &vp_two);

		// Set Primitive Topology
		con->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Update the point light for attenuation
		if (!doFlip)
		{
			lightClr[1].y -= t;
			lightClr[1].z -= t;
			cone -= t;

			if(!XMVector3GreaterOrEqual({ lightClr[1].x, lightClr[1].y, lightClr[1].z }, { 0.0f,0.1f,0.1f }))
				doFlip = true;
		}
		else if (doFlip)
		{
			lightClr[1].y += t;
			lightClr[1].z += t;
			cone += t;
			if (XMVector3GreaterOrEqual({ lightClr[1].x, lightClr[1].y, lightClr[1].z }, { 0.0f, 0.9f, 0.9f }))
				doFlip = false;
		}

		// Rotate the directional light around the origin
		if (!moveDirLight)
		{
			XMMATRIX mRotate = XMMatrixRotationY(0.2f * t);
			XMVECTOR vLightDir = XMLoadFloat4(&lightDir[0]);
			vLightDir = XMVector3Transform(vLightDir, mRotate);
			XMStoreFloat4(&lightDir[0], vLightDir);
		}

		// Rotate the positional light around the origin
		XMMATRIX mRotate = XMMatrixRotationY(-1.0f * t);
		XMVECTOR vLightDir = XMLoadFloat4(&lightDir[1]);
		vLightDir = XMVector3Transform(vLightDir, mRotate);
		XMStoreFloat4(&lightDir[1], vLightDir);

		// Constant Buffer to communicate with the shader's values on the GPU
		ConstantBuffer cb;
		cb.mWorld = XMMatrixTranspose(g_World);
		XMVECTOR det;
		if (flag == 1)
			cb.mView = XMMatrixTranspose(XMMatrixInverse(&det, g_View));
		else
			cb.mView = XMMatrixTranspose(g_View);
		cb.mProjection = XMMatrixTranspose(g_Projection);
		// Directional Light [0]
		cb.lightDir[0] = lightDir[0];
		cb.lightClr[0] = lightClr[0];
		// Point Light [1]
		cb.lightDir[1] = lightDir[1];
		cb.lightClr[1] = lightClr[1];
		// Spot Light [2]
		cb.lightDir[2] = lightDir[2];
		cb.lightClr[2] = lightClr[2];
		cb.vOutputColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		cb.time = tTotal;
		cb.cone = cone;
		con->UpdateSubresource(constantbuffer.Get(), 0, nullptr, &cb, 0, 0);

		// Unique Constant Buffer to communicate for unique PS
		UniqueBuffer ub;
		ub.timePos = { tUpToOne, 0, 0, 0};
		con->UpdateSubresource(u_constantbuffer.Get(), 0, nullptr, &ub, 0, 0);

		// Render the mesh
		// Set vertex buffer
		const UINT stride[] = { sizeof(SimpleVertex) };
		const UINT offset[] = { 0 };
		ID3D11Buffer* const buffs[] = { vertexbuffer.Get() };
		con->IASetVertexBuffers(0, ARRAYSIZE(buffs), buffs, stride, offset);

		// Set Index Buffer
		con->IASetIndexBuffer(indexbuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// Set Vertex Shader
		con->VSSetShader(vertexshader.Get(), nullptr, 0);
		con->VSSetConstantBuffers(0, 1, constantbuffer.GetAddressOf());
		// Set the Geometry Shader
		con->GSSetShader(geoshader.Get(), 0, 0);
		con->GSSetConstantBuffers(0, 1, constantbuffer.GetAddressOf());
		// Set Pixel Shader
		con->PSSetShader(pixelshader.Get(), nullptr, 0);
		con->PSSetConstantBuffers(0, 1, constantbuffer.GetAddressOf());
		con->PSSetShaderResources(0, 1, textureRV.GetAddressOf());
		con->PSSetShaderResources(1, 1, normRV.GetAddressOf());
		con->PSSetSamplers(0, 1, samplerLinear.GetAddressOf());
		// Draw out the mesh
		//con->DrawIndexed(mesh->indicesList.size(), 0, 0);

		// Reset Geometry Shader so it doesn't affect everything else.
		con->GSSetShader(nullptr, 0, 0);

		if(flag == 1)
			DrawBehind(con, view, cb, mesh->indicesList.size());

		// Set Index Buffer
		con->IASetIndexBuffer(indexbuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// Set Vertex Shader
		con->VSSetShader(vertexshader.Get(), nullptr, 0);
		con->VSSetConstantBuffers(0, 1, constantbuffer.GetAddressOf());
		// Set the Geometry Shader
		con->GSSetShader(geoshader.Get(), 0, 0);
		con->GSSetConstantBuffers(0, 1, constantbuffer.GetAddressOf());
		// Set Pixel Shader
		con->PSSetShader(pixelshader.Get(), nullptr, 0);
		con->PSSetConstantBuffers(0, 1, constantbuffer.GetAddressOf());
		con->PSSetShaderResources(0, 1, textureRV.GetAddressOf());
		con->PSSetShaderResources(1, 1, normRV.GetAddressOf());
		con->PSSetSamplers(0, 1, samplerLinear.GetAddressOf());

		// Draw out the mesh
		con->DrawIndexed(mesh->indicesList.size(), 0, 0);

		// Reset Geometry Shader so it doesn't affect everything else.
		con->GSSetShader(nullptr, 0, 0);

		// Render the light sources as cubes (So they are visible)
		// Set vertex buffer
		const UINT c_stride[] = { sizeof(SimpleVertex) };
		const UINT c_offset[] = { 0 };
		ID3D11Buffer* const c_buffs[] = { c_vertexbuffer.Get() };
		con->IASetVertexBuffers(0, ARRAYSIZE(c_buffs), c_buffs, c_stride, c_offset);

		// Set Index Buffer
		con->IASetIndexBuffer(c_indexbuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// Render the lighting sources as a cube.
		for (int i = 0; i < 3; i++)
		{
			// Directional Light
			if (i == 0)
			{
				XMMATRIX mLight = XMMatrixTranslationFromVector(5.0f * XMLoadFloat4(&lightDir[i]));
				XMMATRIX mLightScale = XMMatrixScaling(0.2f, 0.2f, 0.2f);
				mLight = mLightScale * mLight;

				// Update the world variable to reflect the current light
				cb.mWorld = XMMatrixTranspose(mLight);
				cb.vOutputColor = lightClr[i];
				con->UpdateSubresource(constantbuffer.Get(), 0, nullptr, &cb, 0, 0);

				// Update PS's constant buffer to unique
				con->PSSetConstantBuffers(1, 1, u_constantbuffer.GetAddressOf());
				con->PSSetShader(pixelshaderUnique.Get(), nullptr, 0);
			}
			// Positional Light
			else if (i == 1)
			{
				XMMATRIX mLight = XMMatrixTranslationFromVector(1.0f * XMLoadFloat4(&lightDir[i]));
				XMMATRIX mLightScale = XMMatrixScaling(0.05f, 0.05f, 0.05f);
				mLight = mLightScale * mLight;

				// Update the world variable to reflect the current light
				cb.mWorld = XMMatrixTranspose(mLight);
				cb.vOutputColor = lightClr[i];
				con->UpdateSubresource(constantbuffer.Get(), 0, nullptr, &cb, 0, 0);

				// Be sure the constant buffer is still the contsant buffer.
				con->PSSetConstantBuffers(0, 1, constantbuffer.GetAddressOf());
				con->PSSetShader(pixelshaderSolid.Get(), nullptr, 0);
			}
			// Spot Light
			else
			{
				XMVECTOR lightPos = { 0.0f, 2.0f, -2.0f, 1.0f};
				XMMATRIX mLight = XMMatrixTranslationFromVector(1.0f * lightPos);
				XMMATRIX mLightScale = XMMatrixScaling(0.05f, 0.05f, 0.05f);
				mLight = mLightScale * mLight;

				// Update the world variable to reflect the current light
				cb.mWorld = XMMatrixTranspose(mLight);
				cb.vOutputColor = lightClr[i];
				con->UpdateSubresource(constantbuffer.Get(), 0, nullptr, &cb, 0, 0);

				// Be sure the constant buffer is still the contsant buffer.
				con->PSSetConstantBuffers(0, 1, constantbuffer.GetAddressOf());
				con->PSSetShader(pixelshaderSolid.Get(), nullptr, 0);
			}

			con->DrawIndexed(36, 0, 0);
		}

		// Render the Skybox
		{
			XMFLOAT4 skyPos = {0, 0, 0, 0};
			XMMATRIX mSky = XMMatrixTranslationFromVector(1.0f * XMLoadFloat4(&skyPos));
			XMMATRIX mScaleSky = XMMatrixScaling(1.0f, 2.0f, 1.0f);
			mSky = mScaleSky * mSky;

			// Update world variable for skybox
			cb.mWorld = XMMatrixTranspose(mSky);
			cb.vOutputColor = { 1.0f, 1.0f, 1.0f, 1.0f };
			con->UpdateSubresource(constantbuffer.Get(), 0, nullptr, &cb, 0, 0);
			con->PSSetShaderResources(2, 1, SKBtextureRV.GetAddressOf());

			// Update vertex and pixel shader for skybox.
			con->VSSetShader(SKBvertexshader.Get(), nullptr, 0);
			con->VSSetConstantBuffers(0, 1, constantbuffer.GetAddressOf());
			con->PSSetShader(SKBpixelshader.Get(), nullptr, 0);
			con->PSSetConstantBuffers(0, 1, constantbuffer.GetAddressOf());
			
			con->IASetInputLayout(SKBinput.Get());
			con->OMSetDepthStencilState(depthStencilState.Get(), 0);
			con->DrawIndexed(36, 0, 0);
			con->OMSetDepthStencilState(NULL, 0);
		}
		//DrawBehind(con, view, cb, 36);
		con->IASetInputLayout(input.Get());

		// Render the Grid
		RenderGrid(con, view, cb);
		//DrawBehind(con, view, cb, gridIndices.size());

		//renderReflectionCube(con, view, cb);
		
		RenderRTT(con, view, cb, 36);

		// First passthrough for first VP.
		if (flag == 1)
		{
			// Store the current world
			stor_World = g_World;
			vp_two_World = g_World;
			g_World = vp_two_World;
			// And view
			stor_View = g_View;
			g_View = vp_two_View;
			// And projection
			stor_Projection = g_Projection;
			g_Projection = vp_two_Projection;

			// Render with the second VP now.
			Render(2);
		}
		// Second passthrough for second VP.
		else if (flag == 2)
		{
			g_World = stor_World;
			g_View = stor_View;
			g_Projection = stor_Projection;
		}

		timePerFrame = timeCur;

		con->Release();
		view->Release();
	}

	void changePerspective()
	{
		g_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV2 + zoom, DrawClass::width / (FLOAT)DrawClass::height, nearP, farP);
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

		// Toggle the normal mapping in the Pixel Shader
		//if ((GetKeyState('M') & 0x8000) && ghostProtectZ == false)
		//{
		//	zoom += 0.01f;
		//	changePerspective();
		//	ghostProtectZ = true;
		//	std::cout << "Norm Map: " << zoom << '\n';
		//}
		//else if ((GetKeyState('M') == 0x0000) && ghostProtectZ == true)
		//{
		//	zoom += 0.01f;
		//	changePerspective();
		//	ghostProtectZ = false;
		//	std::cout << "Norm Map: " << zoom << '\n';
		//}

		// Left Shift [Zooms in by decreasing FOV]
		if (GetAsyncKeyState(VK_LSHIFT))
		{
			if (zoom > -1.5f)
			{
				zoom -= 0.005f;
				changePerspective();
			}
			// std::cout << "Zoom Level: " << zoom << '\n';
		}
		// Left Control ['Zooms' out by increasing FOV]
		if (GetAsyncKeyState(VK_LCONTROL))
		{
			if (zoom < 0.75f)
			{
				zoom += 0.005f;
				changePerspective();
			}
			// std::cout << "Zoom Level: " << zoom << '\n';
		}

		// Moves near plane closer
		if (GetAsyncKeyState('G'))
		{
			if (nearP > 0.05f)
			{
				nearP -= 0.05f;
				changePerspective();
			}
		}

		// Moves near plane farther
		if (GetAsyncKeyState('T'))
		{
			if (nearP < 50.0f && (nearP + 2.0f) < farP)
			{
				nearP += 0.05f;
				changePerspective();
			}
		}

		// Moves far plane closer
		if (GetAsyncKeyState('Y'))
		{
			if (farP > 10.0f && GetAsyncKeyState('6'))
			{
				farP = 10.0f;	// I added this here so that the effect could be visually seen faster. (Since my scene is small.)
				std::cout << "[NOT AN ERROR] Successfully set far-plane to 10.0f.\n|\n";
			}

			if (farP > (nearP + 2.0f))
			{
				farP -= 0.1f;
				changePerspective();
			}
		}

		// Moves far plane farther
		if (GetAsyncKeyState('H'))
		{
			if (farP < 250.0f)
			{
				farP += 0.1f;
				changePerspective();
			}
		}

		// Reset Zoom & Clipping Planes
		if (GetAsyncKeyState('R'))
		{
			zoom = 0.0f;
			nearP = 0.01f;
			farP = 100.0f;
			changePerspective();
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

			// Block input outside of 125 pixels away from center.
			if (abs(diffX) < 125 && abs(diffY) < 125)
			{
				// Get angle from fwd vector to 
				XMVECTOR fwd = XMVector3Cross(g_World.r[0], g_View.r[1]);
				fwd = XMVector3Normalize(fwd);
				XMVECTOR vec = XMVector3AngleBetweenNormals(g_World.r[1], fwd);
				vec = XMVectorMultiply(vec, { 180 / 3.14f, 180 / 3.14f, 180 / 3.14f });

				// std::cout << XMVectorGetX(vec) << '\n';

				// Make sure the camera isn't looking straight down or upwards
				if (XMVectorGetX(vec) > 10.0f && XMVectorGetX(vec) < 170.0f)
				{
					// Create the rotation matrix based on mouse input.
					XMMATRIX rot = XMMatrixRotationRollPitchYaw(-diffY / 150.0f, -diffX / 150.0f, 0);

					g_View = XMMatrixMultiply(rot, g_View);

					XMVECTOR vExistingZ = g_View.r[2];
					// Parallel to the world's horizon 
					XMVECTOR vNewX = XMVector3Cross(g_World.r[1], vExistingZ);
					XMVECTOR vNewY = XMVector3Cross(vExistingZ, vNewX);
					vExistingZ = XMVector3Normalize(vExistingZ);
					vNewY = XMVector3Normalize(vNewY);
					vNewX = XMVector3Normalize(vNewX);

					XMMATRIX newView = {
						XMVectorGetX(vNewX), XMVectorGetY(vNewX), XMVectorGetZ(vNewX), XMVectorGetW(g_View.r[0]),
						XMVectorGetX(vNewY), XMVectorGetY(vNewY), XMVectorGetZ(vNewY), XMVectorGetW(g_View.r[1]),
						XMVectorGetX(vExistingZ), XMVectorGetY(vExistingZ), XMVectorGetZ(vExistingZ), XMVectorGetW(g_View.r[2]),
						XMVectorGetX(g_View.r[3]), XMVectorGetY(g_View.r[3]), XMVectorGetZ(g_View.r[3]), XMVectorGetW(g_View.r[3])
					};

					g_View = newView;
				}
				// Just do X-Rotation
				else
				{
					// Create the rotation matrix based on mouse input.
					XMMATRIX rot = XMMatrixRotationRollPitchYaw(diffY / 125.0f, diffX / 125.0f, 0);

					g_View = XMMatrixMultiply(rot, g_View);
				}
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
			0, 0, 0.1f, 1
			};
			g_View = XMMatrixMultiply(translate, g_View);
		}

		if (GetAsyncKeyState('S'))
		{
			XMMATRIX translate = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, -0.1f, 1
			};
			g_View = XMMatrixMultiply(translate, g_View);
		}

		if (GetAsyncKeyState('A'))
		{
			XMMATRIX translate = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			-0.1f, 0, 0, 1
			};
			g_View = XMMatrixMultiply(translate, g_View);
		}

		if (GetAsyncKeyState('D'))
		{
			XMMATRIX translate = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0.1f, 0, 0, 1
			};
			g_View = XMMatrixMultiply(translate, g_View);
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
			0, 0.1f, 0, 1
			};
			g_View = XMMatrixMultiply(g_View, translate);
		}

		if (GetAsyncKeyState('C'))
		{
			XMMATRIX translate = {
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, -0.1f, 0, 1
			};
			g_View = XMMatrixMultiply(g_View, translate);
		}
	}

	//void ReleaseOnResize()
	//{
	//	if(renderTargetView.GetAddressOf() != nullptr)
	//		renderTargetView.Get()->Release();
	//	resized = true;
	//}

	//void InitializeOnResize()
	//{
	//	ID3D11Device* dev = nullptr;
	//	ID3D11DeviceContext* con = nullptr;
	//	ID3D11DepthStencilView* depthview = nullptr;
	//	+d3d11.GetDevice((void**)&dev);
	//	+d3d11.GetImmediateContext((void**)(&con));
	//	+d3d11.GetDepthStencilView((void**)&depthview);

	//	// Back Buffer setup
	//	{
	//		IDXGISwapChain* swp = nullptr;
	//		+d3d11.GetSwapchain((void**)(&swp));
	//		// Create a render target view
	//		ID3D11Texture2D* pBackBuffer = nullptr;
	//		if (FAILED(swp->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer))))
	//			return;

	//		if (FAILED(dev->CreateRenderTargetView(pBackBuffer, nullptr, renderTargetView.GetAddressOf())))
	//		{
	//			pBackBuffer->Release();
	//			return;
	//		}
	//		pBackBuffer->Release();
	//		swp->Release();

	//		con->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthview);
	//		depthview->Release();
	//	}
	//}
};