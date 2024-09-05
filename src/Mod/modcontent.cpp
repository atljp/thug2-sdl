/*
TODO: Example CFuncs to be called from modded scripts

Extra tech: Wallride anywhere
if (wallrideanywhere)
{
	patchJump((void*)0x00500441, wallrideanywhere_patch);
	if (wallrideanywhere == 2) patchNop((void*)0x00500424, 6);
}
Log::TypedLog(CHN_DLL, "Wallride Anywhere: %s\n", (wallrideanywhere == 2) ? "Extended" : (wallrideanywhere ? "Enabled" : "Disabled"));

if (jankdrops)
{
	patchNop((void*)0x00502629, 6);
	patchDWord((void*)0x006467BC, 0x3F733333);
}
Log::TypedLog(CHN_DLL, "Jank Drops: %s\n", jankdrops ? "Enabled" : "Disabled");

Log::TypedLog(CHN_DLL, "Custom mods: %s\n", usemod ? "Enabled" : "Disabled");

*/