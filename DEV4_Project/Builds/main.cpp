#include "defines.h"

//Structures//
struct SimpleVertex
{
	XMFLOAT3 Pos;
};

//Globals//
unsigned int width, height;

GWindow win;
GEventReceiver msgs;
GDirectX11Surface d3d11;

// DirectX Globals //
IDXGISwapChain* swap = nullptr;
ID3D11DeviceContext* con = nullptr;
ID3D11RenderTargetView* view = nullptr;
ID3D11InputLayout* input = nullptr;
ID3D11VertexShader* vertexshader = nullptr;
ID3D11PixelShader* pixelshader = nullptr;
ID3D11Buffer* vertexbuffer = nullptr;
D3D11_VIEWPORT vp;
//-------//

HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
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

HRESULT InitDevice()
{
	ID3D11Device* dev = nullptr;
	+d3d11.GetDevice((void**)&dev);
	+d3d11.GetImmediateContext((void**)(&con));

	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	con->RSSetViewports(1, &vp);

	// Compile the vertex shader
	ID3DBlob* pVSBlob = nullptr;
	HRESULT hr = CompileShaderFromFile(L"shaders.fx", "VS", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the vertex shader
	hr = dev->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &vertexshader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	// Create the input layout
	hr = dev->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &input);
	pVSBlob->Release();
	if (FAILED(hr))
		return hr;

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
	hr = CompileShaderFromFile(L"shaders.fx", "PS", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the pixel shader
	hr = dev->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &pixelshader);
	pPSBlob->Release();
	if (FAILED(hr))
		return hr;

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
	hr = dev->CreateBuffer(&bd, &InitData, &vertexbuffer);
	if (FAILED(hr))
		return hr;

	// Set vertex buffer
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	
	con->IASetVertexBuffers(0, 1, &vertexbuffer, &stride, &offset);

	// Set primitive topology
	con->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	dev->Release();
	con->Release();
	return S_OK;
}

// lets pop a window and use D3D11 to clear to a green screen
int main()
{
	if (+win.Create(0, 0, 800, 600, GWindowStyle::WINDOWEDBORDERED))
	{
		win.SetWindowName("DEV4_Project");

		width = +win.GetWidth(width);
		height = +win.GetHeight(height);

		float clr[] = { 0.2f, 0.2f, 0.8f, 1 }; // start with blue
		msgs.Create(win, [&]() {
			if (+msgs.Find(GWindow::Events::RESIZE, true))
				{
					width = +win.GetWidth(width);
					height = +win.GetHeight(height);
					clr[0] += 0.01f; // move towards a cyan as they resize
				}
			});
		if (+d3d11.Create(win, 0))
		{
			if (FAILED(InitDevice()))
			{
				return 0;
			}

			while (+win.ProcessWindowEvents())
			{
				if (+d3d11.GetImmediateContext((void**)&con) &&
					+d3d11.GetRenderTargetView((void**)&view) &&
					+d3d11.GetSwapchain((void**)&swap))
				{
					con->ClearRenderTargetView(view, clr);

					// Render a triangle
					con->VSSetShader(vertexshader, nullptr, 0);
					con->PSSetShader(pixelshader, nullptr, 0);
					con->Draw(3, 0);

					swap->Present(0, 0);
					// release incremented COM reference counts
					swap->Release();
					view->Release();
					con->Release();
				}
			}
		}
	}
	return 0; // that's all folks
}