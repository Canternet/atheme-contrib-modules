/*
 * Copyright (c) 2015 Colgate Minuette <colgate@canternet.org>
 * Rights to this code are as documented in doc/LICENSE.
 *
 * Set/unset channel mode +P on registration/deregistration.
 *
 * Shamelessly reused code from cs_regmode.c
 */

#include "atheme-compat.h"
#include "protocol/inspircd.h"
#include "channels.h"

DECLARE_MODULE_V1
(
	"contrib/cs_permmode", false, _modinit, _moddeinit,
        PACKAGE_STRING,
        "Colgate Minuette colgate@canternet.org"
);

static void os_cmd_perm(sourceinfo_t *si, int parc, char *parv[]);

command_t os_perm = { "PERM", "Makes all registered channels permanent. (+P)", PRIV_OMODE, 2, os_cmd_perm, { .path = "contrib/cs_permmode" } };

static void register_hook(hook_channel_req_t *hdata)
{
	mychan_t *mc = hdata->mc;

	if (mc == NULL || mc->chan == NULL)
		return;

	modestack_mode_simple(chansvs.nick, mc->chan, MTYPE_ADD, CMODE_PERM);
}

static void drop_hook(mychan_t *mc)
{
	if (mc == NULL || mc->chan == NULL)
		return;

	modestack_mode_simple(chansvs.nick, mc->chan, MTYPE_DEL, CMODE_PERM);
}

static void os_cmd_perm(sourceinfo_t *si, int parc, char *parv[])
{
	channel_t *c;
	mowgli_patricia_iteration_state_t state;
	int count = 0;
	char *modeparv[256];
	char *mode = "+P";
	
	int modeparc = sjtoken(mode, ' ', modeparv);

	MOWGLI_PATRICIA_FOREACH(c, &state, chanlist)
	{
		if((mychan_find(c->name)))
		{
			channel_mode(si->service->me, c, modeparc, modeparv);
			count++;
		}
	}
	command_success_nodata(si, _("Set mode +P on \2%d\2 channels."), count);
	wallops("\2%s\2 is using PERM",get_oper_name(si));
	logcommand(si, CMDLOG_ADMIN, "PERM");

}

void
_modinit(module_t *m)
{
	hook_add_event("channel_register");
	hook_add_channel_register(register_hook);
	service_named_bind_command("operserv", &os_perm);

	hook_add_event("channel_drop");
	hook_add_channel_drop(drop_hook);
}

void
_moddeinit(module_unload_intent_t intent)
{
	hook_del_channel_register(register_hook);
	hook_del_channel_drop(drop_hook);
	service_named_unbind_command("operserv", &os_perm);
}
