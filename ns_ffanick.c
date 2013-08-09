/*
 * Copyright (c) 2013 Googolplexed
 * Modified from os_akillnicklist.c by William Pitcock
 * Licence is same as above code


 *Copyright (c) 2005-2013 Atheme Project (http://www.atheme.org)
 *Portions copyright (c) 2004 Shrike Developers (http://www.malkier.net/)
 *
 *Permission to use, copy, modify, and/or distribute this software for any
 *purpose with or without fee is hereby granted, provided that the above
 *copyright notice and this permission notice appear in all copies.
 *
 *THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 *ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Deny registration/grouping of a specific set of nicks
 *
 * Basically this builds a keyword patricia.  O(NICKLEN) lookups at the price
 * of a longer startup process.
 *
 * Configuration.
 * ==============
 *
 * This module adds a new block to the config file:
 *
 *     freenicks {
 *         nicklist = "/home/googol/_atheme/etc/freenicks.txt";
 *     };
 *
 */

#include "atheme-compat.h"

DECLARE_MODULE_V1
(
	"contrib/ns_ffanick", false, _modinit, _moddeinit,
	"0.1",
	"Googolplexed"
);

static mowgli_patricia_t *forbidnicklist = NULL;

static mowgli_list_t conft = { NULL, NULL, 0 };

static void
add_contents_of_file_to_list(const char *filename, mowgli_patricia_t *list)
{
	char value[BUFSIZE];
	FILE *f;

	f = fopen(filename, "r");
	if (!f)
		return;

	while (fgets(value, BUFSIZE, f) != NULL)
	{
		strip(value);

		if (!*value)
			continue;

		mowgli_patricia_add(list, value, (void *) 0x1);
	}

	fclose(f);
}

static int
nicklist_config_handler(mowgli_config_file_entry_t *entry)
{
	add_contents_of_file_to_list(entry->vardata, forbidnicklist);

	return 0;
}

static void
nickcheck_hook(hook_user_register_check_t *data)
{
	const char *nick;

	return_if_fail(data != NULL);
	
	nick = data->account;

	if (mowgli_patricia_retrieve(forbidnicklist, nick) == NULL)
		return;

	command_fail(data->si,fault_noprivs, _("The nick \2%s\2 is a canon nick, and cannot be registered, as it is reserved for free use by everyone"),nick);
	data->approved++;
}

void
_modinit(module_t *m)
{
	add_subblock_top_conf("freenicks", &conft);
	add_conf_item("nicklist", &conft, nicklist_config_handler);

	forbidnicklist = mowgli_patricia_create(strcasecanon);

	hook_add_event("nick_can_register");
	hook_add_nick_can_register(nickcheck_hook);
}

void
_moddeinit(module_unload_intent_t intent)
{
	hook_del_nick_can_register(nickcheck_hook);

	del_conf_item("nicklist", &conft);
	del_top_conf("freenicks");
}
