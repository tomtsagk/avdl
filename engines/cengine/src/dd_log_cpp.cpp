#include "dd_log.h"
#include <stdarg.h>
#include <stdio.h>

#if defined(AVDL_DIRECT3D11)

#include <windows.h>

using namespace Windows::UI::Popups;

void dd_log(const char *msg, ...) {

	va_list args;
	va_start(args, msg);

	char buffer[1024];
	vsnprintf(buffer, 1024, msg, args);

	//std::string s_str = std::string(buffer);
	//std::wstring wid_str = std::wstring(s_str.begin(), s_str.end());
	//const wchar_t* w_char = wid_str.c_str();
	//Platform::String^ p_string = ref new Platform::String(w_char);

	MessageDialog^ dialog = ref new MessageDialog("Test dialog");
	UICommand^ continueCommand = ref new UICommand("Ok");
	UICommand^ upgradeCommand = ref new UICommand("Cancel");

	dialog->DefaultCommandIndex = 0;
	dialog->CancelCommandIndex = 1;
	dialog->Commands->Append(continueCommand);
	dialog->Commands->Append(upgradeCommand);
	dialog->ShowAsync();

	va_end(args);
}

#endif
