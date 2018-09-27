#include "std_include.hpp"

#include "cef/cef_ui.hpp"
#include "cef/cef_ui_app.hpp"
#include "cef/cef_ui_handler.hpp"

using namespace literally;

namespace gameoverlay
{
	void cef_ui::ui_runner()
	{
		dynlib proc;
		CefMainArgs args(proc.get_handle());

		CefSettings settings;
		settings.no_sandbox = TRUE;
		//settings.single_process = TRUE;
		settings.windowless_rendering_enabled = TRUE;
		settings.pack_loading_disabled = FALSE;
		settings.remote_debugging_port = 12345;

#ifdef DEBUG
		settings.log_severity = LOGSEVERITY_VERBOSE;

#else
		settings.log_severity = LOGSEVERITY_DISABLE;
#endif

		CefString(&settings.browser_subprocess_path) = this->path + "process.exe";
		CefString(&settings.locales_dir_path) = this->path + "cef\\locales";
		CefString(&settings.resources_dir_path) = this->path + "cef";
		CefString(&settings.log_file) = this->path + "cef_data\\debug.log";
		CefString(&settings.user_data_path) = this->path + "cef_data\\user";
		CefString(&settings.cache_path) = this->path + "cef_data\\cache";
		CefString(&settings.locale) = "en-US";

		CefInitialize(args, settings, this->get_app(), 0);

		CefBrowserSettings browser_settings;
		browser_settings.windowless_frame_rate = 60;
		browser_settings.web_security = STATE_DISABLED;

		std::string url = "http://google.com";

		CefWindowInfo window_info;
		window_info.SetAsWindowless(GetDesktopWindow());
		window_info.windowless_rendering_enabled = TRUE;

		this->browser = CefBrowserHost::CreateBrowserSync(window_info, this->get_client(), url, browser_settings, NULL);

		CefRunMessageLoop();
		CefShutdown();
	}

	CefRefPtr<cef_ui_app> cef_ui::get_app()
	{
		return this->app;
	}

	CefRefPtr<cef_ui_handler> cef_ui::get_client()
	{
		return this->client;
	}

	void cef_ui::invoke_close_browser(CefRefPtr<CefBrowser> browser)
	{
		if (!browser) return;
		browser->GetHost()->CloseBrowser(true);
	}

	void cef_ui::close_browser()
	{
		if (!this->browser) return;
		CefPostTask(TID_UI, base::Bind(&cef_ui::invoke_close_browser, this->browser));
		this->browser = nullptr;
	}

	cef_ui::cef_ui()
	{
		dynlib self = dynlib::get_by_address(dynlib::get_by_address);
		this->path = self.get_folder();

		dynlib::add_load_path(this->path + "cef");
		dynlib libcef("libcef.dll", true);
		if (!libcef.is_valid() || !libcef.delay_import())
		{
			throw std::runtime_error("Unable to import libcef");
		}

		CefEnableHighDPISupport();

		this->app = new cef_ui_app();
		this->client = new cef_ui_handler();
		this->ui_thread = std::thread(std::bind(&cef_ui::ui_runner, this));
	}

	cef_ui::~cef_ui()
	{
		if (this->browser)
		{
			this->close_browser();
		}

		if (this->ui_thread.joinable())
		{
			this->ui_thread.join();
		}
	}
}
