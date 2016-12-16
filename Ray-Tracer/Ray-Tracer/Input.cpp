#include "Input.h"
#include "stdafx.h"

		Input::Input():
			_mousePosX(0),
			_mousePosY( 0),
			_lastMousePosX( 0),
			_lastMousePosY( 0),

			_mouseLockedToScreen(false),
			_mouseLockedToCenter(false)
		{
			memset(_keys, 0, sizeof(_keys));
			memset(_keyPressed, 0, sizeof(_keyPressed));
		}


		Input::~Input()
		{

		}
		const void Input::Frame()
		{
			POINT p;

			GetCursorPos(&p);

			ScreenToClient(_hwnd, &p);


			_mousePosX = (int)p.x;
			_mousePosY = (int)p.y;

			_xDiff = _lastMousePosX - _mousePosX;
			_yDiff = _lastMousePosY - _mousePosY;



			if (false)//fullscreen
			{
				_width = GetSystemMetrics(SM_CXSCREEN);
				_height = GetSystemMetrics(SM_CYSCREEN);
			}
			if (_mouseLockedToCenter)
			{
				_lastMousePosX = _mousePosX = _width / 2;
				_lastMousePosY = _mousePosY = _height / 2;

				p.x = _mousePosX;
				p.y = _mousePosY;
				ClientToScreen(_hwnd, &p);


				SetCursorPos(p.x, p.y);
			}
			else
			{
				_lastMousePosX = _mousePosX;
				_lastMousePosY = _mousePosY;
			}

			_scrollDelta = 0;
			_keys[Keys::ScrollUp] = false;
			_keys[Keys::ScrollDown] = false;

			for (uint32_t i = 0; i < NUM_KEYS; i++)
				//if (_keys[i])
					_keyPressed[i] = false;

			return void();
		}
		const bool Input::IsKeyDown(Keys key) const
		{
			auto findkey = _rebinds.find(key);
			if (findkey != _rebinds.end())
			{
				key = findkey->second;
			}

			return _keys[key];
		}
		const bool Input::IsKeyPushed(Keys key)
		{
			auto findkey = _rebinds.find(key);
			if (findkey != _rebinds.end())
			{
				key = findkey->second;
			}

			return _keyPressed[key];

		}
		const float Input::GetScrollDelta() const
		{
			return (float)_scrollDelta;
		}
		const void Input::GetMousePos(int32_t & rX, int32_t & rY) const
		{
		//	auto o = System::GetOptions();
			if (false)
			{
				/*float sw = (float)GetSystemMetrics(SM_CXSCREEN);
				float sh = (float)GetSystemMetrics(SM_CYSCREEN);
				float ww = (float)o->GetScreenResolutionWidth();
				float wh = (float)o->GetScreenResolutionHeight();
				float pw = ww / sw;
				float ph = wh / sh;
				rX = static_cast<int>(_mousePosX*pw);
				rY = static_cast<int>(_mousePosY*ph);*/


			}
			else
			{
				rX = _mousePosX;
				rY = _mousePosY;
			}
			return void();
		}
		const void Input::GetMouseDiff(int32_t & rX, int32_t & rY) const
		{
			rX = _xDiff;
			rY = _yDiff;
			return void();
		}
		const void Input::LockMouseToCenter(bool lock)
		{
			if (lock)
			{
				//auto o = System::GetOptions();
				uint32_t wW = _width;
				uint32_t wH = _height;

				//if (Utils::Options::GetBooleanOption("Screen", "Fullscreen", false))
				//{
				//	wW = GetSystemMetrics(SM_CXSCREEN);
				//	wH = GetSystemMetrics(SM_CYSCREEN);
				//}
				RECT r;
				GetWindowRect(_hwnd, &r);
				uint32_t wX = r.left;
				uint32_t wY = r.top;

				_lastMousePosX = wW / 2;
				_lastMousePosY = wH / 2;
				_mousePosX = wW / 2;
				_mousePosY = wH / 2;

				RECT rc = { 0,0,0,0 };

				AdjustWindowRect(&rc, GetWindowStyle(_hwnd), FALSE);

				SetCursorPos(wX + _mousePosX - rc.left, wY + _mousePosY - rc.top);
			}
			_mouseLockedToCenter = lock;
		}
		const void Input::LockMouseToWindow(bool lock)
		{
			if (lock)
			{
				//auto o = System::GetOptions();
				RECT clipping;
				clipping.left = 0;
				clipping.right = _width;
				clipping.top = 0;
				clipping.bottom = _height;
				//if (Utils::Options::GetBooleanOption("Screen", "Fullscreen", false))
				//{
				//	clipping.right = GetSystemMetrics(SM_CXSCREEN);
				//	clipping.bottom = GetSystemMetrics(SM_CYSCREEN);
				//	ClipCursor(&clipping);
				//}
				//else
				//{
					RECT rc = clipping;
					AdjustWindowRect(&rc, GetWindowStyle(_hwnd), FALSE);

					RECT rcClip;           // new area for ClipCursor

					GetWindowRect(_hwnd, &rcClip);
					rcClip.right -= rc.right - clipping.right;
					rcClip.bottom -= rc.bottom - clipping.bottom;
					rcClip.left -= rc.left - clipping.left;
					rcClip.top -= rc.top - clipping.top;
					// Confine the cursor to the application's window. 

					ClipCursor(&rcClip);
				//}
			}
			else
			{
				ClipCursor(nullptr);
			}
			_mouseLockedToScreen = lock;
		}
		const void Input::HideCursor(bool show) const
		{
			if (show)
			{
				while (ShowCursor(false) >= 0);
			}
			else
				while (ShowCursor(true) < 0);
		}
		const void Input::Rebind(Keys key, Keys to)
		{
			_rebinds[key] = to;
			return void();
		}
		const void Input::Init(HWND hwnd, int width, int height)
		{
			_hwnd = hwnd;
			_width = width;
			_height = height;


			_lastMousePosX = _mousePosX = _width / 2;
			_lastMousePosY = _mousePosY = _height / 2;
			RECT rc;
			POINT p;
			GetWindowRect(_hwnd, &rc);
			uint32_t wX = rc.left;
			uint32_t wY = rc.top;
			p.x = _mousePosX;
			p.y = _mousePosY;
			ClientToScreen(_hwnd, &p);

			SetCursorPos(p.x, p.y);

			return void();
		}
		LRESULT Input::MessageHandler(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam)
		{
			switch (umsg)
			{
			case WM_MOUSEMOVE:
				_OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
				break;
			case WM_LBUTTONDOWN:
				_KeyDown(Keys::LButton);
				break;
			case WM_LBUTTONUP:
				_KeyUp(Keys::LButton);
				break;
			case WM_RBUTTONDOWN:
				_KeyDown(Keys::RButton);
				break;
			case WM_RBUTTONUP:
				_KeyUp(Keys::RButton);
				break;
			case WM_MBUTTONDOWN:
				_KeyDown(Keys::MButton);
				break;
			case WM_MBUTTONUP:
				_KeyUp(Keys::MButton);
				break;
			case WM_MOUSEWHEEL:
				_OnMouseScroll(GET_WHEEL_DELTA_WPARAM(wParam));
				break;
			case WM_XBUTTONDOWN:
				switch (GET_XBUTTON_WPARAM(wParam))
				{
				case XBUTTON1:
					_KeyDown(Keys::XButton1);
					break;

				case XBUTTON2:
					_KeyDown(Keys::XButton2);
					break;
				}
				break;		
			case WM_XBUTTONUP:
				switch (GET_XBUTTON_WPARAM(wParam))
				{
				case XBUTTON1:
					_KeyUp(Keys::XButton1);
					break;

				case XBUTTON2:
					_KeyUp(Keys::XButton2);
					break;
				}
				break;
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
				_KeyDown(_MapLeftRightKeys( wParam, lParam));
				break;
			case WM_KEYUP:
			case WM_SYSKEYUP:
				_KeyUp(_MapLeftRightKeys(wParam, lParam));
				break;
			case WM_KILLFOCUS:
				memset(_keys, 0, sizeof(_keys));
				memset(_keyPressed, 0, sizeof(_keyPressed));
				break;
			case WM_INPUT:
			default:
			{
				return DefWindowProc(hwnd, umsg, wParam, lParam);
			}

			}
			return 0;
			
		}
		const void Input::_KeyDown(Keys key)
		{
			_keys[key] = true;
			_keyPressed[key] = true;
			return void();
		}
		const void Input::_KeyUp(Keys key)
		{
			_keyPressed[key] = _keys[key] = false;
			return void();
		}
		const void Input::_OnMouseMove(uint32_t x, uint32_t y)
		{
			_mousePosX = x;
			_mousePosY = y;
			return void();
		}
		const void Input::_OnMouseScroll(int32_t delta)
		{
			if (delta > 0)
				_keys[Keys::ScrollUp] = true;
			else if (delta < 0)
				_keys[Keys::ScrollDown] = true;
			_scrollDelta = delta;
			return void();
		}

		WPARAM Input::_MapLeftRightKeys(WPARAM vk, LPARAM lParam)
		{
			WPARAM new_vk = vk;
			UINT scancode = (lParam & 0x00ff0000) >> 16;
			int extended = (lParam & 0x01000000) != 0;

			switch (vk) {
			case VK_SHIFT:
				new_vk = MapVirtualKey(scancode, MAPVK_VSC_TO_VK_EX);
				break;
			case VK_CONTROL:
				new_vk = extended ? VK_RCONTROL : VK_LCONTROL;
				break;
			case VK_MENU:
				new_vk = extended ? VK_RMENU : VK_LMENU;
				break;
			default:
				// not a key we map from generic to left/right specialized
				//  just return it.
				new_vk = vk;
				break;
			}

			return new_vk;
		}
