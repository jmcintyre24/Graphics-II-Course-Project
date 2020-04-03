#include "DrawClass.h"
#include <zmouse.h>

DrawClass::DrawClass(GW::GRAPHICS::GDirectX11Surface _d3d11, GW::SYSTEM::GWindow _win)
{
	win = _win;
	d3d11 = _d3d11;
	+win.GetWidth(width);
	+win.GetHeight(height);
}
