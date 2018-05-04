#pragma once

#include <irenderer.hpp>

namespace gameoverlay
{
	class cef_ui_app;

	class cef_ui_handler : public CefClient, public CefDisplayHandler, public CefLifeSpanHandler, public CefRenderHandler, public CefLoadHandler, public CefContextMenuHandler
	{
	public:
        explicit cef_ui_handler(CefRefPtr<cef_ui_app> app);
		~cef_ui_handler();

		virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() override
		{
			return this;
		}

		virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override
		{
			return this;
		}

		virtual CefRefPtr<CefRenderHandler> GetRenderHandler() override
		{
			return this;
		}

		virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override
		{
			return this;
		}

		virtual CefRefPtr<CefContextMenuHandler> GetContextMenuHandler() override
		{
			return this;
		}

		virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message) override;

		virtual bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;
		virtual void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height) override;

		virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
		virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

		virtual void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model) override;
		virtual bool OnContextMenuCommand(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, int command_id, CefContextMenuHandler::EventFlags event_flags) override;
		virtual void OnContextMenuDismissed(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame) override;
		virtual bool RunContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model, CefRefPtr<CefRunContextMenuCallback> callback) override;

		void closeAllBrowsers(bool forceClose);
		void set_canvas(icanvas* canvas_ptr);
		void trigger_resize();

	private:
		icanvas* canvas;

		CefRefPtr<cef_ui_app> app;
		std::vector<CefRefPtr<CefBrowser>> browserList;

		IMPLEMENT_REFCOUNTING(cef_ui_handler);
	};
}