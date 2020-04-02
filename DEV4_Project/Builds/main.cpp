#include "defines.h"

#include "DrawClass.h"

using namespace GW;
using namespace CORE;
using namespace SYSTEM;
using namespace GRAPHICS;

//Globals//
unsigned int width, height;

GWindow win;
GEventReceiver msgs;
GDirectX11Surface d3d11;

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
			//Triangle tri(d3d11, win);
			Cube cub(d3d11, win);
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