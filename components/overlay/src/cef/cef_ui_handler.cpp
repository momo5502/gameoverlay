#include "std_include.hpp"

#include "cef/cef_ui.hpp"
#include "cef/cef_ui_handler.hpp"

namespace gameoverlay
{
	cef_ui_handler::cef_ui_handler()
	{

	}

	cef_ui_handler::~cef_ui_handler()
	{

	}

	void cef_ui_handler::OnAfterCreated(CefRefPtr<CefBrowser> browser)
	{
		CEF_REQUIRE_UI_THREAD();
		browser_list.push_back(browser);
	}

	void cef_ui_handler::OnBeforeClose(CefRefPtr<CefBrowser> browser)
	{
		CEF_REQUIRE_UI_THREAD();

		for (auto bit = this->browser_list.begin(); bit != this->browser_list.end(); ++bit)
		{
			if ((*bit)->IsSame(browser))
			{
				browser_list.erase(bit);
				break;
			}
		}

		if (browser_list.empty())
		{
			CefQuitMessageLoop();
		}
	}

	void cef_ui_handler::OnBeforeContextMenu(CefRefPtr<CefBrowser> /*browser*/, CefRefPtr<CefFrame> /*frame*/, CefRefPtr<CefContextMenuParams> /*params*/, CefRefPtr<CefMenuModel> model)
	{
		model->Clear();
	}

	bool cef_ui_handler::OnContextMenuCommand(CefRefPtr<CefBrowser> /*browser*/, CefRefPtr<CefFrame> /*frame*/, CefRefPtr<CefContextMenuParams> /*params*/, int /*command_id*/, CefContextMenuHandler::EventFlags /*event_flags*/)
	{
		return false;
	}

	void cef_ui_handler::OnContextMenuDismissed(CefRefPtr<CefBrowser> /*browser*/, CefRefPtr<CefFrame> /*frame*/)
	{

	}

	bool cef_ui_handler::RunContextMenu(CefRefPtr<CefBrowser> /*browser*/, CefRefPtr<CefFrame> /*frame*/, CefRefPtr<CefContextMenuParams> /*params*/, CefRefPtr<CefMenuModel> /*model*/, CefRefPtr<CefRunContextMenuCallback> /*callback*/)
	{
		return false;
	}

	void cef_ui_handler::close_all_browsers(bool force_close)
	{
		if (this->browser_list.empty()) return;

		if (!CefCurrentlyOn(TID_UI))
		{
			CefPostTask(TID_UI, base::Bind(&cef_ui_handler::close_all_browsers, this, force_close));
			return;
		}

		for (auto& browser : this->browser_list)
		{
			browser->GetHost()->CloseBrowser(force_close);
		}
	}

	void cef_ui_handler::set_canvas(icanvas* canvas_ptr)
	{
		this->canvas = canvas_ptr;
	}

	void cef_ui_handler::trigger_resize()
	{
		for (auto& browser : this->browser_list)
		{
			browser->GetHost()->WasResized();
		}
	}

	bool cef_ui_handler::GetViewRect(CefRefPtr<CefBrowser> /*browser*/, CefRect& rect)
	{
		if (this->canvas)
		{
			rect.Set(0, 0, this->canvas->get_width(), this->canvas->get_height());
		}
		else
		{
			rect.Set(0, 0, 640, 480);
		}

		return true;
	}

	void cef_ui_handler::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType /*type*/, const RectList& /*dirty_rects*/, const void* buffer, int width, int height)
	{
		//Components::XUI::UpdateTexture(buffer, width, height);
		if (!this->canvas) return;
		if (this->canvas->get_width() == uint32_t(width) && this->canvas->get_height() == uint32_t(height))
		{
			this->canvas->paint(buffer);
		}
		else
		{
			browser->GetHost()->WasResized();
		}
	}

	bool cef_ui_handler::OnProcessMessageReceived(CefRefPtr<CefBrowser> /*browser*/, CefProcessId /*sourceProcess*/, CefRefPtr<CefProcessMessage> /*message*/)
	{
		return false;
	}
}
