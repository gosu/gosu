// This should be a short #ifdef'ed section in RubyGosu.swg, but due to three
// involved namespace hells (Ruby!!!, SWIG, Carbon), this is impossible.

#include <Carbon/Carbon.h>

extern "C" {
void Gosu_MacStartup()
{
	// This is for Ruby/Gosu and misc. hackery:
	// Usually, applications on the Mac can only get keyboard and mouse input if
	// run by double-clicking an .app. So if this is run from the Terminal (i.e.
	// during Ruby/Gosu game development), tell the OS we need input in any case.
	ProcessSerialNumber psn = { 0, kCurrentProcess };
	TransformProcessType(&psn, kProcessTransformToForegroundApplication);
    SetFrontProcess(&psn);
}
}