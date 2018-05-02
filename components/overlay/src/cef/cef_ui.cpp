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

		CefRefPtr<cef_ui_app> cefApplication(new cef_ui_app());

		CefSettings cSettings;
		cSettings.no_sandbox = TRUE;
		cSettings.single_process = TRUE;
		cSettings.windowless_rendering_enabled = TRUE;
		cSettings.pack_loading_disabled = FALSE;
		cSettings.remote_debugging_port = 12345;

#ifdef DEBUG
		cSettings.log_severity = LOGSEVERITY_VERBOSE;

#else
		cSettings.log_severity = LOGSEVERITY_DISABLE;
#endif

		CefString(&cSettings.browser_subprocess_path) = proc.get_path();
		CefString(&cSettings.locales_dir_path) = ".\\cef\\locales";
		CefString(&cSettings.resources_dir_path) = proc.get_folder() + "cef";
		CefString(&cSettings.log_file) = ".\\cef\\debug.log";
		CefString(&cSettings.locale) = "en-US";

		CefInitialize(args, cSettings, cefApplication, 0);
		CefRunMessageLoop();
		CefShutdown();
	}

	cef_ui::cef_ui()
	{
		dynlib::add_load_path("cef");
		dynlib libcef("libcef.dll", true);
		if (!libcef.is_valid() || !libcef.delay_import())
		{
			throw std::runtime_error("Unable to import libcef");
		}

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
