#include "std_include.hpp"

#include "cef/cef_ui.hpp"
#include "cef/cef_ui_app.hpp"

using namespace literally;

namespace gameoverlay
{
	void cef_ui::ui_runner()
	{
		dynlib proc;
		CefMainArgs args(proc.get_handle());

		CefSettings settings;
		settings.no_sandbox = TRUE;
		settings.single_process = TRUE;
		settings.windowless_rendering_enabled = TRUE;
		settings.pack_loading_disabled = FALSE;
		settings.remote_debugging_port = 12345;

#ifdef DEBUG
		settings.log_severity = LOGSEVERITY_VERBOSE;

#else
		settings.log_severity = LOGSEVERITY_DISABLE;
#endif

		CefString(&settings.browser_subprocess_path) = proc.get_path();
		CefString(&settings.locales_dir_path) = ".\\cef\\locales";
		CefString(&settings.resources_dir_path) = proc.get_folder() + "cef";
		CefString(&settings.log_file) = ".\\cef\\debug.log";
		CefString(&settings.locale) = "en-US";

		CefInitialize(args, settings, this->get_app(), 0);
		CefRunMessageLoop();
		CefShutdown();
	}

	CefRefPtr<cef_ui_app> cef_ui::get_app()
	{
		return this->app;
	}

	cef_ui::cef_ui()
	{
		dynlib::add_load_path("cef");
		dynlib libcef("libcef.dll", true);
		if (!libcef.is_valid() || !libcef.delay_import())
		{
			throw std::runtime_error("Unable to import libcef");
		}

		this->app = new cef_ui_app();
		this->ui_thread = std::thread(std::bind(&cef_ui::ui_runner, this));

		CefEnableHighDPISupport();
	}

	cef_ui::~cef_ui()
	{
		if (this->ui_thread.joinable())
		{
			this->ui_thread.join();
		}
	}
}
