#include "/DEV4/Graphics-II-Course-Project//DEV4_Project//Builds/defines.h"

void InitDevice(GDirectX11Surface& d3d11)
{
	IDXGISwapChain* swap;
	ID3D11DeviceContext* con;
	ID3D11RenderTargetView* view;
	if (+d3d11.GetImmediateContext((void**)&con) &&
		+d3d11.GetRenderTargetView((void**)&view) &&
		+d3d11.GetSwapchain((void**)&swap))
	{
		con->ClearRenderTargetView(view, clr);
		swap->Present(1, 0);
		// release incremented COM reference counts
		swap->Release();
		view->Release();
		con->Release();
	}
}

// lets pop a window and use D3D11 to clear to a green screen
int main()
{
	GWindow win;
	GEventReceiver msgs;
	GDirectX11Surface d3d11;
	if (+win.Create(0, 0, 800, 600, GWindowStyle::WINDOWEDBORDERED))
	{
		win.SetWindowName("DEV_4 Project");

		float clr[] = { 255/255.0f, 1.0f, 20/255.0f, 1 };
		msgs.Create(win, [&]() {
			if (+msgs.Find(GWindow::Events::RESIZE, true))
				clr[2] += 0.01f;
			});
		
		if (+d3d11.Create(win, 0))
		{
			while (+win.ProcessWindowEvents())
			{
				IDXGISwapChain* swap;
				ID3D11DeviceContext* con;
				ID3D11RenderTargetView* view;
				if (+d3d11.GetImmediateContext((void**)&con) &&
					+d3d11.GetRenderTargetView((void**)&view) &&
					+d3d11.GetSwapchain((void**)&swap))
				{
					con->ClearRenderTargetView(view, clr);
					swap->Present(1, 0);
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