/*
 net-nonblock.c : Nonblocking net_connect()

    Copyright (C) 1998-2000 Timo Sirainen

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "module.h"

#include <signal.h>

#include <irssi/src/core/network.h>
#include <irssi/src/core/net-nonblock.h>

typedef struct {
	GResolverNameLookupFlags flags;
	NetGethostbynameContinuationFunc cont;
	void *cont_data;
} NET_GETHOSTBYNAME_CALLBACK_DATA;

static void net_gethostbyname_callback(GResolver *resolver, GAsyncResult *result,
                                       NET_GETHOSTBYNAME_CALLBACK_DATA *data)
{
	/* GList<GInetAddress> */
	GList *ailist;
	GError *error;
	RESOLVED_IP_REC *iprec;

	error = NULL;
#if GLIB_CHECK_VERSION(2, 59, 0)
	ailist = g_resolver_lookup_by_name_with_flags_finish(resolver, result, &error);
#else
	/* compatibility code for old GLib */
	ailist = g_resolver_lookup_by_name_finish(resolver, result, &error);
	if (error == NULL && data->flags) {
		GList *ll, *lll;

		for (ll = ailist; ll != NULL; ll = lll) {
			GInetAddress *address;
			GSocketFamily family;

			address = G_INET_ADDRESS(ll->data);
			family = g_inet_address_get_family(address);
			lll = ll->next;

			if ((data->flags == G_RESOLVER_NAME_LOOKUP_FLAGS_IPV4_ONLY &&
			     family == G_SOCKET_FAMILY_IPV6) ||
			    (data->flags == G_RESOLVER_NAME_LOOKUP_FLAGS_IPV6_ONLY &&
			     family == G_SOCKET_FAMILY_IPV4)) {
				g_object_unref(address);
				ailist = g_list_delete_link(ailist, ll);
			}
		}

		if (ailist == NULL) {
			g_set_error(&error, G_RESOLVER_ERROR, G_RESOLVER_ERROR_NOT_FOUND,
			            data->flags == G_RESOLVER_NAME_LOOKUP_FLAGS_IPV4_ONLY ?
			                "IPv4 address not found for host" :
			                "IPv6 address not found for host");
		}
	}
#endif
	iprec = g_new0(RESOLVED_IP_REC, 1);
	if (error != NULL) {
		iprec->error = error;
	} else {
		iprec->ailist = ailist;
	}
	g_object_unref(resolver);
	resolved_ip_ref(iprec);

	data->cont(iprec, data->cont_data);
	g_free(data);
}

/* nonblocking gethostbyname() */
GCancellable *net_gethostbyname_nonblock(const char *addr, GResolverNameLookupFlags flags,
                                         NetGethostbynameContinuationFunc cont, void *cont_data)
{
	GResolver *resolver;
	GCancellable *cancellable;
	NET_GETHOSTBYNAME_CALLBACK_DATA *data;

	g_return_val_if_fail(addr != NULL, FALSE);

<<<<<<< HEAD
	pid = fork();
	if (pid > 0) {
		/* parent */
		pidwait_add(pid);
		return pid;
	}

	if (pid != 0) {
		/* failed! */
		g_warning("net_connect_thread(): fork() failed! "
			  "Using blocking resolving");
	}

	memset(&rec, 0, sizeof(rec));
	rec.error = net_gethostbyname(addr, &rec.ip4, &rec.ip6);
	if (rec.error == 0) {
		errorstr = NULL;
	} else {
		errorstr = net_gethosterror(rec.error);
		rec.errlen = errorstr == NULL ? 0 : strlen(errorstr)+1;
	}

	i_io_channel_write_block(pipe, &rec, sizeof(rec));
	if (rec.errlen != 0)
		i_io_channel_write_block(pipe, (void *) errorstr, rec.errlen);

	if (pid == 0)
		_exit(99);

	/* we used blocking lookup */
	return 0;
}

/* get the resolved IP address */
int net_gethostbyname_return(GIOChannel *pipe, RESOLVED_IP_REC *rec)
{
	rec->error = -1;
	rec->errorstr = NULL;

	fcntl(g_io_channel_unix_get_fd(pipe), F_SETFL, O_NONBLOCK);

	/* get ip+error */
	if (i_io_channel_read_block(pipe, rec, sizeof(*rec)) == -1) {
		rec->errorstr = g_strdup_printf("Host name lookup: %s",
						g_strerror(errno));
		return -1;
	}

	if (rec->error) {
		/* read error string, if we can't read everything for some
		   reason, just ignore it. */
		rec->errorstr = g_malloc0(rec->errlen+1);
		i_io_channel_read_block(pipe, rec->errorstr, rec->errlen);
	}

	return 0;
}

/* Kill the resolver child */
void net_disconnect_nonblock(int pid)
{
	g_return_if_fail(pid > 0);

	kill(pid, SIGKILL);
=======
	resolver = g_resolver_get_default();
	cancellable = g_cancellable_new();
	data = g_new0(NET_GETHOSTBYNAME_CALLBACK_DATA, 1);
	data->flags = flags;
	data->cont = cont;
	data->cont_data = cont_data;
#if GLIB_CHECK_VERSION(2, 59, 0)
	g_resolver_lookup_by_name_with_flags_async(resolver, addr, flags, cancellable,
	                                           (GAsyncReadyCallback) net_gethostbyname_callback,
	                                           data);
#else
	/* compatibility code for old GLib */
	g_resolver_lookup_by_name_async(resolver, addr, cancellable,
	                                (GAsyncReadyCallback) net_gethostbyname_callback, data);
#endif
	return cancellable;
>>>>>>> upstream/master
}
