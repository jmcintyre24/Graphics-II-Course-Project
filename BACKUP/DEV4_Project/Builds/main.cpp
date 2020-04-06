#include "defines.h"

#include "DrawClass.h"
#include "StoneHenge.h"

using namespace GW;
using namespace CORE;
using namespace SYSTEM;
using namespace GRAPHICS;

//Globals//
unsigned int width, height;

GWindow win;
GEventReceiver msgs;
GDirectX11Surface d3d11;

void ReadModel(Mesh::SimpleMesh& mesh)
{
	// Read Vertex Data In
	for (int i = 0; i < 1457; i++)
	{
		//Position
		Mesh::SimpleVertex vert;
		vert.Pos = { (StoneHenge_data[i].pos[0] * 0.1f),
			(StoneHenge_data[i].pos[1] * 0.1f),
			(StoneHenge_data[i].pos[2] * 0.1f) };

		//UV		   
		vert.UV = { StoneHenge_data[i].uvw[0],
				StoneHenge_data[i].uvw[1] };

		//Normal	   
		vert.Normal = { (StoneHenge_data[i].nrm[0]),
			(StoneHenge_data[i].nrm[1]),
			(StoneHenge_data[i].nrm[2]) };

		//For now render random color.
		//vert.Color = { 1.0f, 1.0f, 1.0f, 1.0f };

		mesh.vertexList.push_back(vert);
	}

	// Read Indicies in - COULD USE MEMCOPY HERE
	for (int i = 0; i < 2532; i++)
	{
		mesh.indicesList.push_back(StoneHenge_indicies[i]);
	}
}

void PrintInstructions()
{
	std::cout << "~~Controls~~ \n"
		<< "(TOGGLE)Z -> (HOLD)Left-Mouse -> Move Directional Light\n"
		<< "(HOLD)Right-Mouse - Look Around\n"
		<< "WASD - General Camera Movement\n"
		<< "C - Travel Downwards\n"
		<< "SPACE - Travel Upwards\n"
		<< "Q\E - Quickly turn left and right\n"
		<< "~~~~~~~~~~ERRORS BELOW THIS LINE~~~~~~~~~~\n\n";
}

// lets pop a window and use D3D11 to clear to a green screen
int main()
{
	if (+win.Create(0, 0, 800, 600, GWindowStyle::WINDOWEDBORDERED))
	{
		win.SetWindowName("DEV4_Project");

		+win.GetWidth(width);
		+win.GetHeight(height);

		float clr[] = { 0.2f, 0.2f, 0.8f, 1 }; // start with blue
		msgs.Create(win, [&]() {
			if (+msgs.Find(GWindow::Events::RESIZE, true))
				{
					+win.GetWidth(width);
					+win.GetHeight(height);
					clr[0] += 0.01f; // move towards a cyan as they resize
				}
			});
		if (+d3d11.Create(win, 0))
		{
			Mesh::SimpleMesh mesh;

			PrintInstructions();
			ReadModel(mesh);

			//Triangle tri(d3d11, win);
			Mesh cub(d3d11, win, &mesh);
			while (+win.ProcessWindowEvents())
			{
				IDXGISwapChain* swap = nullptr;
				ID3D11DeviceContext* con = nullptr;
				ID3D11RenderTargetView* view = nullptr;

				if (+d3d11.GetImmediateContext((void**)&con) &&
					+d3d11.GetRenderTargetView((void**)&view) &&
					+d3d11.GetSwapchain((void**)&swap))
				{
					con->ClearRenderTargetView(view, clr);

					cub.UserInput();
					//tri.Render();
					cub.Render();
					
					swap->Present(1, 0);
					con->Release();
					view->Release();
					swap->Release();
				}
			}
		}
	}
	return 0;
}