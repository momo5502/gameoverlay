#pragma once

namespace gameoverlay
{
	class cef_ui_handler;

	class cef_ui_app : public CefApp, public CefBrowserProcessHandler, public CefRenderProcessHandler
	{
	public:
		cef_ui_app();

		CefRefPtr<cef_ui_handler> get_client();

		virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override
		{
			return this;
		}

		virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override
		{
			return this;
		}

		virtual void OnContextInitialized() override;
		virtual void OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) override;

	protected:
		virtual void OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line) override;
		virtual void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) override;
		virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message) override;


	private:
		CefRefPtr<cef_ui_handler> client;

		IMPLEMENT_REFCOUNTING(cef_ui_app);
	};
}
